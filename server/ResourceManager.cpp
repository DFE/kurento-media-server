/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ResourceManager.hpp"

#include <gst/gst.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <KurentoException.hpp>
#include <MediaSet.hpp>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

#define GST_CAT_DEFAULT kurento_resource_manager
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoResourceManager"

namespace kurento
{

#ifdef __linux__
static long int
get_int (std::string &str, char sep, int nToken)
{
  size_t start = str.find_first_not_of (sep), end;
  int count = 0;

  while (start != std::string::npos) {
    end = str.find (sep, start);

    if (count++ == nToken) {
      str[end] = '\0';
      return atol (&str.c_str() [start]);
    }

    start = str.find_first_not_of (sep, end);
  }

  return 0;
}
#endif

static long int
getNumberOfThreads ()
{
#ifdef _WIN32
  // See https://stackoverflow.com/questions/3749668/how-to-query-the-thread-count-of-a-process-using-the-regular-windows-c-c-apis
  DWORD  const   pid        = GetCurrentProcessId ();
  HANDLE const   snapshot   = CreateToolhelp32Snapshot (TH32CS_SNAPALL, 0);
  PROCESSENTRY32 entry      = { 0 };
  entry.dwSize              = sizeof (entry);
  
  BOOL ret = Process32First (snapshot, &entry);

  while (ret && entry.th32ProcessID != pid) {
	  ret = Process32Next (snapshot, &entry);
  }

  CloseHandle (snapshot);
  // in case of an error, simply count this thread
  return ret ? entry.cntThreads : 1;
#else
  std::string stat;
  std::ifstream stat_file ("/proc/self/stat");  // `man proc`

  std::getline (stat_file, stat);
  stat_file.close();

  return get_int (stat, ' ', 19);
#endif
}

rlim_t
getMaxThreads ()
{
  static rlim_t limit = 0;

  if (limit == 0) {
#ifdef _WIN32
    // Safe assumption based on
    // https://docs.microsoft.com/en-us/archive/blogs/markrussinovich/pushing-the-limits-of-windows-processes-and-threads
    limit = (rlim_t)2000;
#else
    struct rlimit limits;
    getrlimit (RLIMIT_NPROC, &limits);

    limit = limits.rlim_cur;
#endif
  }

  return limit;
}

static void
checkThreads (float limit_percent)
{
  const rlim_t maxThreads = getMaxThreads ();
  if (maxThreads <= 0 || maxThreads == RLIM_INFINITY) {
    return;
  }

  const rlim_t maxThreadsKms = (rlim_t)(maxThreads * limit_percent);
  const rlim_t nThreads = (rlim_t)getNumberOfThreads ();

  if (nThreads > maxThreadsKms) {
    std::ostringstream oss;
    oss << "Reached KMS threads limit: " << maxThreadsKms;
    std::string exMessage = oss.str();

    oss << " (system max: " << maxThreads << ");"
        << " set a higher limit with `ulimit -Su`, or in the KMS service settings (/etc/default/kurento-media-server)";
    std::string logMessage = oss.str();

    GST_WARNING ("%s", logMessage.c_str());
    throw KurentoException (NOT_ENOUGH_RESOURCES, exMessage);
  }
}

rlim_t
getMaxOpenFiles ()
{
  static rlim_t limit = 0;

  if (limit == 0) {
#ifdef _WIN32
	static bool increased = false;
	if (!increased) {
		// Set to maximum possible.
		// See https://stackoverflow.com/questions/1803552/setmaxstdio-max-open-files-is-2048-only
		_setmaxstdio(2048);
		increased = true;
	}
    limit = (rlim_t)_getmaxstdio();
#else
    struct rlimit limits;
    getrlimit (RLIMIT_NOFILE, &limits);

    limit = limits.rlim_cur;
#endif
  }

  return limit;
}

static long int
getNumberOfOpenFiles ()
{
  long int openFiles = 0;
#ifdef _WIN32
  // GetProcessHandleCount() returns the number of kernel handles
  // which is significantly higher than what the user process has opened.
  // GetGuiResources() seems to work even without having an application
  // window (despite the API description of Microsoft).
  openFiles = (long int) GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
#else
  DIR *d;
  struct dirent *dir;

  d = opendir ("/proc/self/fd");

  while ((dir = readdir(d)) != nullptr) {
    openFiles ++;
  }

  closedir (d);
#endif

  return openFiles;
}

static void
checkOpenFiles (float limit_percent)
{
  const rlim_t maxOpenFiles = getMaxOpenFiles ();
  if (maxOpenFiles <= 0 || maxOpenFiles == RLIM_INFINITY) {
    return;
  }

  const rlim_t maxOpenFilesKms = (rlim_t)(maxOpenFiles * limit_percent);
  const rlim_t nOpenFiles = (rlim_t)getNumberOfOpenFiles ();

  if (nOpenFiles > maxOpenFilesKms) {
    std::ostringstream oss;
    oss << "Reached KMS files limit: " << maxOpenFilesKms;
    std::string exMessage = oss.str();

    oss << " (system max: " << maxOpenFiles << ");"
        << " set a higher limit with `ulimit -Sn`, or in the KMS service settings (/etc/default/kurento-media-server)";
    std::string logMessage = oss.str();

    GST_WARNING ("%s", logMessage.c_str());
    throw KurentoException (NOT_ENOUGH_RESOURCES, exMessage);
  }
}

void
checkResources (float limit_percent)
{
  checkThreads (limit_percent);
  checkOpenFiles (limit_percent);
}

void killServerOnLowResources (float limit_percent)
{
  MediaSet::getMediaSet()->signalEmptyLocked.connect ([limit_percent] () {
    GST_DEBUG ("MediaSet empty, checking resources");

    try {
      checkResources (limit_percent);
    } catch (KurentoException &e) {
      if (e.getCode() == NOT_ENOUGH_RESOURCES) {
        GST_ERROR ("Resources over the limit, server will be killed: %s",
            e.what());
#ifdef _WIN32
        exit (1);
#else
        kill ( getpid(), SIGTERM );
#endif
      }
    }
  });
}

} /* kurento */

static void init_debug() __attribute__((constructor));

static void init_debug() {
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
