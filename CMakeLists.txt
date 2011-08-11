# Copyright 2011 Tomas Härdin
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.6)
project(james)

add_subdirectory(libjames)

add_executable(james
    main.cpp
    Class.cpp
    BuiltInClasses.cpp
)

target_link_libraries(james libjames)

# Account for the Xerces-C++ 3.1.x library being called xerces-c_3 on WIN32
if(WIN32)
    target_link_libraries(james xerces-c_3)
else(WIN32)
    target_link_libraries(james xerces-c)
endif(WIN32)

install(TARGETS james DESTINATION bin)

