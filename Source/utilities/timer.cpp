 //
 //  timer.cpp
 //  
 //
 //  Created by Andreas Grosam on 8/25/11.
 //  Copyright 2011 Andreas Grosam
 //
 //  Licensed under the Apache License, Version 2.0 (the "License");
 //  you may not use this file except in compliance with the License.
 //  You may obtain a copy of the License at
 //
 //  http://www.apache.org/licenses/LICENSE-2.0
 //
 //  Unless required by applicable law or agreed to in writing, software
 //  distributed under the License is distributed on an "AS IS" BASIS,
 //  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 //  See the License for the specific language governing permissions and
 //  limitations under the License.
 //

#include "timer.hpp"


namespace utilities {
    uint64_t timer::s_correction = 0;
}