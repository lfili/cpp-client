cmake_minimum_required (VERSION 2.6)

project (HotRod C CXX)

#file(READ version.txt HR_VERSION_FILE)
#string(REGEX MATCHALL "[0-9]+" HR_VERSION_LIST "${HR_VERSION_FILE}")

#list(GET HR_VERSION_LIST 0 HR_VERSION_MAJOR)
#list(GET HR_VERSION_LIST 1 HR_VERSION_MINOR)

#set (HR_VERSION "${HR_VERSION_MAJOR}.${HR_VERSION_MINOR}")
#message(STATUS "HR_VERSION: ${HR_VERSION}")

enable_testing()
include (CTest)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")

# MacOS has a bunch of differences in build tools and process and so we have to turn some things
# off there by default (for GCC; LLVM will need flags re-evaluated)
if (APPLE)
  set (NOENABLE_WARNING_ERROR ON)
  set (NOENABLE_UNDEFINED_ERROR ON)
  set (NOENABLE_SWIG_TESTING ON)
endif (APPLE)

# Add options here called <whatever> they will turn into "ENABLE_<whatever" and can be
# overridden on a platform specific basis above by NOENABLE_<whatever>
set (OPTIONS WARNING_ERROR UNDEFINED_ERROR SWIG_TESTING)

foreach (OPTION ${OPTIONS})
  if (NOT "NOENABLE_${OPTION}")
    set ("DEFAULT_${OPTION}" ON)
  endif (NOT "NOENABLE_${OPTION}")
endforeach (OPTION)

# And add the option here too with help text
option(ENABLE_WARNING_ERROR "Consider compiler warnings to be errors" ${DEFAULT_WARNING_ERROR})
option(ENABLE_UNDEFINED_ERROR "Check for unresolved library symbols" ${DEFAULT_UNDEFINED_ERROR})
option(ENABLE_SWIG_TESTING "Create SWIG Java binding and test structure" ${DEFAULT_SWIG_TESTING})


if (CMAKE_COMPILER_IS_GNUCXX)
  if (ENABLE_WARNING_ERROR)
    set (WERROR "-Werror")
  endif (ENABLE_WARNING_ERROR)

  set (COMPILER_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
  set (WARNING_FLAGS
        "${WERROR} -pedantic -Wall -Wextra -Wno-shadow -Wpointer-arith -Wcast-qual -Wcast-align -Wno-long-long -Wvolatile-register-var -Winvalid-pch -Wno-system-headers -Woverloaded-virtual -Wshadow")
  if (ENABLE_UNDEFINED_ERROR)
    set (CATCH_UNDEFINED "-Wl,--no-undefined")
  endif (ENABLE_UNDEFINED_ERROR)

endif (CMAKE_COMPILER_IS_GNUCXX)

if (MSVC)
   set (COMPILER_FLAGS "")
   set (WARNING_FLAGS "")
endif (MSVC)

if (NOT DEFINED COMPILER_FLAGS)
    message(FATAL_ERROR "Compiler flags not set for this build type")
endif (NOT DEFINED COMPILER_FLAGS)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS} ${WARNING_FLAGS}")

if(WIN32 AND NOT CYGWIN)
  # linking against Windows native libraries, including mingw
  set (HOTROD_WINAPI TRUE)
  set (platform_libs ws2_32 Rpcrt4)
elseif (APPLE)
  # OSX does not need rt
  set (platform_libs pthread)
else (WIN32 AND NOT CYGWIN)
  set (platform_libs pthread rt)
endif(WIN32 AND NOT CYGWIN)

# Select driver
if(HOTROD_WINAPI)
  set (platform_sources src/hotrod/sys/windows/Socket.cpp src/hotrod/sys/windows/Thread.cpp
           src/hotrod/sys/windows/platform.cpp)
else(HOTROD_WINAPI)
  set (platform_sources src/hotrod/sys/posix/Socket.cpp src/hotrod/sys/posix/Thread.cpp
           src/hotrod/sys/posix/platform.cpp src/hotrod/sys/posix/Mutex.cpp)
endif(HOTROD_WINAPI)



add_library (
  hotrod SHARED
  src/hotrod/api/RemoteCacheManager.cpp
  src/hotrod/api/RemoteCacheBase.cpp
  src/hotrod/api/exceptions.cpp
  src/hotrod/impl/configuration/Configuration.cpp
  src/hotrod/impl/configuration/ServerConfiguration.cpp
  src/hotrod/impl/configuration/ConnectionPoolConfiguration.cpp
  src/hotrod/impl/configuration/ServerConfigurationBuilder.cpp
  src/hotrod/impl/configuration/ConfigurationBuilder.cpp
  src/hotrod/impl/RemoteCacheManagerImpl.cpp
  src/hotrod/impl/RemoteCacheImpl.cpp
  src/hotrod/impl/operations/OperationsFactory.cpp
  src/hotrod/impl/operations/PingOperation.cpp
  src/hotrod/impl/operations/GetOperation.cpp
  src/hotrod/impl/operations/PutOperation.cpp
  src/hotrod/impl/operations/PutIfAbsentOperation.cpp
  src/hotrod/impl/operations/ReplaceOperation.cpp
  src/hotrod/impl/operations/RemoveOperation.cpp
  src/hotrod/impl/operations/ContainsKeyOperation.cpp
  src/hotrod/impl/operations/ReplaceIfUnmodifiedOperation.cpp
  src/hotrod/impl/operations/RemoveIfUnmodifiedOperation.cpp
  src/hotrod/impl/operations/GetWithMetadataOperation.cpp
  src/hotrod/impl/operations/BulkGetOperation.cpp
  src/hotrod/impl/operations/BulkGetKeysOperation.cpp
  src/hotrod/impl/operations/StatsOperation.cpp
  src/hotrod/impl/operations/ClearOperation.cpp
  src/hotrod/impl/operations/FaultTolerantPingOperation.cpp
  src/hotrod/impl/protocol/HeaderParams.cpp
  src/hotrod/impl/protocol/Codec12.cpp
  src/hotrod/impl/protocol/CodecFactory.cpp
  src/hotrod/impl/transport/AbstractTransport.cpp
  src/hotrod/impl/transport/tcp/Socket.cpp
  src/hotrod/impl/transport/tcp/TcpTransport.cpp
  src/hotrod/impl/transport/tcp/TcpTransportFactory.cpp
  src/hotrod/impl/transport/tcp/TransportObjectFactory.cpp
  src/hotrod/impl/transport/tcp/RoundRobinBalancingStrategy.cpp
  src/hotrod/sys/Runnable.cpp
  ${platform_sources}
  )
target_link_libraries (hotrod ${platform_libs})
set_target_properties (
  hotrod
  PROPERTIES
  LINK_FLAGS "${CATCH_UNDEFINED}")


# TESTS

# TODO: decide on cmake 2.6 or 2.8, which differ greatly in FindJava
# and FindJNI needed for SWIG-ed Java bindings.  The following
# suffices for running the Hot Rod server to support native C++
# testing:

find_package(Java)
if (NOT JAVA_RUNTIME)
    message(FATAL_ERROR "Java javac compiler not found")
endif (NOT JAVA_RUNTIME)


set(HOTROD_JBOSS_HOME $ENV{JBOSS_HOME} CACHE FILEPATH "Infinispan HOME dir")
if (NOT IS_DIRECTORY ${HOTROD_JBOSS_HOME})
    message(FATAL_ERROR "you must set the JBOSS_HOME environment variable or use -DHOTROD_JBOSS_HOME=/the/path")
endif (NOT IS_DIRECTORY ${HOTROD_JBOSS_HOME})

if (NOT ((EXISTS "${HOTROD_JBOSS_HOME}/bin/standalone.sh") AND (EXISTS "${HOTROD_JBOSS_HOME}/bin/standalone.bat")))
    message(FATAL_ERROR "JBOSS_HOME ${HOTROD_JBOSS_HOME} does not have needed startup scripts")
endif (NOT ((EXISTS "${HOTROD_JBOSS_HOME}/bin/standalone.sh") AND (EXISTS "${HOTROD_JBOSS_HOME}/bin/standalone.bat")))

add_executable (simple test/Simple.cpp)
target_link_libraries (simple hotrod)

add_executable (unit_test test/Unit.cpp)
target_link_libraries (unit_test hotrod)

add_test (unit_test unit_test)
add_test (start_server python ${CMAKE_CURRENT_SOURCE_DIR}/test/server_ctl.py start ${JAVA_RUNTIME} ${HOTROD_JBOSS_HOME} single)
add_test (simple simple)
add_test (stop_server python ${CMAKE_CURRENT_SOURCE_DIR}/test/server_ctl.py stop)

if (ENABLE_SWIG_TESTING)
    include(test/swig/swig.cmake)
endif (ENABLE_SWIG_TESTING)

