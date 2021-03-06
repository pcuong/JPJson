//
//  JsonContainerTest.cpp
//
//  Created by Andreas Grosam on 4/26/11.
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

#include "json/value/value.hpp"
#include <gtest/gtest.h>

// for testing
#include <typeinfo>
#include <sstream>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp> 

#if defined (BOOST_STRICT_CONFIG)
#warning BOOST_STRICT_CONFIG defined
#endif

using namespace json;



namespace {
    
    
    typedef json::value<>   Value;
    
    typedef Value::null_type Null;
    typedef Value::boolean_type Boolean;
    typedef Value::float_number_type FloatNumber;
    typedef Value::integral_number_type IntNumber;
    typedef Value::string_type String;
    typedef Value::object_type Object;
    typedef Value::array_type  Array;
    
    
    // The fixture for testing class json::Value:
    
    class JsonContainerTest : public ::testing::Test {
    protected:
        
        JsonContainerTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonContainerTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
    
    TEST_F(JsonContainerTest, Null) {
        Null n;
        n = null;
    }

    //
    // Variant
    // 
    
        
        
    
    /*
    
    TEST_F(JsonContainerTest, VariantInternal) {
        Value::value_type v_1 = false;
        EXPECT_EQ(true, boost::get<Boolean>(&v_1)!=0);
        
        Value::value_type v_2 = true;
        EXPECT_EQ(true, boost::get<Boolean>(&v_2)!=0);
        Value::value_type v_3 = 0;
        EXPECT_EQ(true, boost::get<Number>(&v_3)!=0);

        Value::value_type v_4 = 1;
        EXPECT_EQ(true, boost::get<Number>(&v_4)!=0);
        
        Value::value_type v_5 = -1.0;
        EXPECT_EQ(true, boost::get<Number>(&v_5)!=0);
        //Value::value_type v_6 = "abc";
        //EXPECT_EQ(true, boost::get<String>(&v_6)!=0);        
    }
    
    
    
    
    TEST_F(JsonContainerTest, ValueNullCtors) {
        // A value can be constructed from any value convertible to instances of any of the variant's type:  
        Null aNull;
        Value v0(aNull); // Null
        EXPECT_EQ(true, v0.is_type<Null>());
        Value v1( (Null()) ); // Null  //note: in order to disambiguate the ctor with a function declarator, we need extra parentheses
        EXPECT_EQ(true, v1.is_type<Null>());        
        Value v2(null);  // this is the constant json::null
        EXPECT_EQ(true, v2.is_type<Null>());
    }

    TEST_F(JsonContainerTest, ValueNumberCtors) {
        EXPECT_EQ(true, Value(1).is_type<Number>());
        EXPECT_EQ(true, Value(1UL).is_type<Number>());
        EXPECT_EQ(true, Value(1ULL).is_type<Number>());
        EXPECT_EQ(true, Value(-1L).is_type<Number>());
        EXPECT_EQ(true, Value(-1LL).is_type<Number>());
        EXPECT_EQ(true, Value(1.0e-3).is_type<Number>());
    }
    
    
    TEST_F(JsonContainerTest, NumberVariant) {
        Number n;
        Value v = n;
        std::stringstream ss;
        ss << n;
        EXPECT_EQ("NaN", ss.str());
    }
    */
}  // namespace
