#ifndef JSON_STRING_BUFFER_HPP
#define JSON_STRING_BUFFER_HPP

//
//  string_buffer.hpp
//
//  Created by Andreas Grosam on 5/16/11.
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

#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <limits>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/identity.hpp>
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversions.hpp"



namespace json 
{
    namespace internal 
    {
        
        using unicode::UTF_8_encoding_tag;
        using unicode::UTF_16_encoding_tag;
        using unicode::UTF_16LE_encoding_tag;
        using unicode::UTF_16BE_encoding_tag;
        using unicode::UTF_32_encoding_tag;
        using unicode::UTF_32LE_encoding_tag;
        using unicode::UTF_32BE_encoding_tag;
        using unicode::platform_encoding_tag;
        using unicode::utf8_code_unit;
        using unicode::utf16_code_unit;
        using unicode::utf32_code_unit;

        
        
        template <typename EncodingT> 
        class string_buffer_base {
        public:
            typedef typename EncodingT::code_unit_type  code_unit_t;
            typedef          code_unit_t*               buffer_type;
            typedef size_t                              size_type;
            typedef typename host_endianness::type      from_endian_t;            
            typedef typename EncodingT::endian_tag      to_endian_t;
            
        private:
            buffer_type         auto_buffer_;
            const size_type     auto_buffer_size_;
            buffer_type         buffer_start_;
            buffer_type         buffer_end_;
            buffer_type         p_;
            
            
            // Currently, string_buffer requires that its encodings's endianness
            // is the same as the host endianness. 
            BOOST_STATIC_ASSERT( (boost::mpl::or_<
                                    boost::is_same<EncodingT, unicode::UTF_8_encoding_tag>,
                                    boost::is_same<from_endian_t, to_endian_t>
                                  >::value) );
            
        public:    
            explicit string_buffer_base(buffer_type autoBuffer, size_type autoBufferSize)
            :   auto_buffer_(autoBuffer), 
                auto_buffer_size_(autoBufferSize), 
                buffer_start_(autoBuffer),
                p_ (autoBuffer),
                buffer_end_(autoBuffer + autoBufferSize)
            {
            }
        private:
            string_buffer_base();  // no default c-tor
            string_buffer_base(const string_buffer_base&);  // no copy c-tor
            string_buffer_base& operator=(const string_buffer_base&);  // no assignsment operator
            
            void throwGrowError() {
                throw std::logic_error("could not grow buffer");
            }
            
            void throwTooBigError() {
                throw std::runtime_error("string too large");
            }
            
        public:
            ~string_buffer_base() {
                if (buffer_start_ != auto_buffer_)
                    free(buffer_start_);
            }
            size_type size() const { return p_ - buffer_start_; }
            size_type capazity() const { return buffer_end_ - buffer_start_; }
            
            bool reset(size_type capazity) {
                if (buffer_start_ != auto_buffer_) {
                    free(buffer_start_);
                    buffer_start_ = 0;
                }
                p_ = 0;
                buffer_end_ = 0;
                if (capazity > auto_buffer_size_) {
                    buffer_start_ = (buffer_type)valloc(capazity * sizeof(code_unit_t));
                    if (buffer_start_) {
                        p_ = buffer_start_;
                        buffer_end_ = buffer_start_ + capazity;
                    }
                } else {
                    buffer_start_ = auto_buffer_;
                    p_ = auto_buffer_;
                    buffer_end_ = auto_buffer_ + auto_buffer_size_;
                }
                return buffer_start_ != 0;
            }
            
            void reset() {
                if (buffer_start_ != auto_buffer_)
                    free(buffer_start_);
                buffer_start_ = auto_buffer_;
                p_ = auto_buffer_;
                buffer_end_ = auto_buffer_ + auto_buffer_size_;
            }
            
            size_type left() const { return buffer_end_ - p_; }
            bool avail() const { return buffer_end_ != p_; }
            
            void reserve(size_t size) {
                if (left() < size) {
                    if (not grow(size - left())) {
                        throwGrowError();
                    }
                }
            }
            
            // Appends a code unit. Does not check the validity of the code unit
            // nor the validity of the code unit in the context of the string.
            void append(code_unit_t v) {
                if (__builtin_expect(p_ == buffer_end_, 0)) {
                    if (not grow(size() + 1)) {
                        throwGrowError();
                    }
                }                
                *p_++ = v;
            }
            
            // Appends a sequence of code units to its buffer. The function does 
            // not check the validity of this sequence nor does it check the 
            // validity of this sequence in the context of the existing string.
            // (currently not endian aware)
            void append(const code_unit_t* p, size_type len) {
                if (len > (std::numeric_limits<size_type>::max()>>3)) {
                    // on 32-bit, this is roughly 1GByte
                    throwTooBigError();
                } 
                size_type required = size() + len;
                if (left() < required) {
                    if (not grow(required)) {
                        throwGrowError();
                    }
                }                
                memcpy(p_, p, len*sizeof(code_unit_t)); //std::copy(p, p + len, p_); 
                p_ += len;
            }
            
            
            // Appends an ASCII character to its internal buffer. The value
            // of ch shall be in the range of valid ASCII characters, that is
            // [0 .. 0x7F]. The function does not check if the character is
            // actually valid.
            // (currently not endian aware)
            void append_ascii(char ch)  { 
                assert(ch <= 0x7F);
                append(static_cast<code_unit_t>(ch));
            }
            
            
            // Appends a zero terminated UTF-8 (including ASCII) encoded string 
            // to the buffer.
            //
            // !! append_cstr() does not perform any checks for the validity  !!
            // !! of the input encoding. If the input encoding is mal-formed, !! 
            // !! result of the operation and its behavior is undefined!      !!
            //
            // If the buffer itself is UTF-8 encoded, the content of cstr is 
            // copied verbatime. Otherwise, unchecked unicode conversion is 
            // applied to convert from the cstr into the buffer's encoding.
            // Returns the number of appended code units to the buffer.
            size_type append_cstr(const char* cstr, std::size_t len) {
                if (len > (std::numeric_limits<size_t>::max()>>3))
                    throwTooBigError();
                return append_cstr_impl<EncodingT>(cstr, len);
            }
            size_type append_cstr(const char* cstr) {
                size_t len = strlen(cstr);
                return append_cstr(cstr, len);
            }            
            
            // Appends a zero terminated string encoded in encoding to the
            // buffer.
            // Its assumed the string is well-formed and endianness is in
            // host endianness.
            size_type append_str(const utf8_code_unit* str) {
                return append_utf_str_impl(str, UTF_8_encoding_tag());
            }                        
//            size_type append_str(const utf16_code_unit* str) {
//                typedef unicode::to_host_endianness<UTF_16_encoding_tag>::type StringEncoding;
//                return append_utf_str_impl(str, StringEncoding());
//            }                        
//            size_type append_str(const utf32_code_unit* str) {
//                typedef unicode::to_host_endianness<UTF_32_encoding_tag>::type StringEncoding;
//                return append_utf_str_impl(str, StringEncoding());
//            }                        

            // Appends a string encoded in encoding with length len to the
            // buffer.
            // Its assumed the string is well-formed and endianness is in
            // host endianness.
            size_type append_str(const utf8_code_unit* str, std::size_t len) {
                return append_utf_str_impl(str, len, UTF_8_encoding_tag());
            }            
//            size_type append_str(const utf16_code_unit* str, std::size_t len) {
//                typedef unicode::to_host_endianness<UTF_16_encoding_tag>::type StringEncoding;
//                return append_utf_str_impl(str, len, StringEncoding());
//            }                        
//            size_type append_str(const utf32_code_unit* str, std::size_t len) {
//                typedef unicode::to_host_endianness<UTF_32_encoding_tag>::type StringEncoding;
//                return append_utf_str_impl(str, len, StringEncoding());
//            }                        
            
            
            // Converts the Unicode code point to its internal encoding and 
            // pushes the result to its buffer.
            // Uses "unsafe" conversion rules. The validity of the code point
            // shall be checked by the caller. The function may not reliably
            // detect error conditions.
            // Returns the number of code units appended.
            size_type append_unicode(json::unicode::code_point_t codepoint) 
            {
                reserve(size() + EncodingT::buffer_size);
                int len = json::unicode::convert_one_unsafe(codepoint, p_, EncodingT());
                assert(p_ <= buffer_end_);
                if (len <= 0) {
                    return 0; // TODO: need thread specific error info for unicode::convert functions
                } else {
                    return len;
                }
            }
            
            // Appends the Unicode replacement character U+FFFD to its buffer.
            // Returns the number of code units appended.
            size_type append_unicode_replacement_character() {
                return append_unicode(0xFFFD);
            }
            
            // Appends a zero character to the end of the buffer to indicate
            // end of string for zero terminated strings. If there is already
            // a zero character at the end, it does nothing.
            // Returns true if a zero character has been appended.
            bool terminate_if() {
                if (size() > 0 and *(p_ - 1) == 0) {
                    return false;  // did not require adding a termination 
                } else {
                    append_ascii(code_unit_t(0));
                    return true;
                }
            }
            
            const code_unit_t* buffer() const { return buffer_start_; }
            
        private:
            
            template <typename Encoding>
            size_t append_cstr_impl(const char* str, std::size_t len,
                             typename boost::enable_if<boost::is_base_of<UTF_8_encoding_tag, Encoding>
                             >::type* = 0 ) 
            {
                append(reinterpret_cast<typename UTF_8_encoding_tag::code_unit_type const*>(str), len);
                return len;
            }
            

            // Append an ASCII string (which is a well-formed UTF-8 string) to 
            // the internal buffer encoded in UTF-16/UTF-32
            template <typename Encoding>
            size_t append_cstr_impl(const char* str, std::size_t len,
                             typename boost::enable_if<
                                  boost::mpl::or_<
                                    boost::is_base_of<UTF_16_encoding_tag, Encoding>,
                                    boost::is_base_of<UTF_32_encoding_tag, Encoding>
                                  >
                             >::type* = 0 ) 
            {
                const char* first = str;
#if !defined (DEBUG)
                std::size_t length = strlen(str);
                this->reserve(this->size() + length);
                first = str;
                // UTF-8 encoding seems to be much safer, and not that much
                // slower than just plain copying the ASCII sequence to the 
                // buffer.
                int error = 0;
                std::size_t count = unicode::convert_unsafe(first, first + len, UTF_8_encoding_tag(), 
                                                            p_, Encoding(), error);
                assert(error == 0);
#else
                int error = 0;
                std::size_t length = unicode::count(first, first + len, UTF_8_encoding_tag(), Encoding(), error);
                if (length != strlen(str)) {
                    throw std::logic_error("input string not well formed UTF-8");
                }
                std::size_t count = 0;
                if (error == 0) {
                    this->reserve(this->size() + length);
                    first = str;
                    count = unicode::convert(first, first + len, UTF_8_encoding_tag(), p_, Encoding(), error);
                }
                
                if (error != 0 or count != length) {
                    throw std::logic_error("input string not well formed UTF-8");
                }
#endif                                
                return count;
            }
            
            
            template <typename StringEncoding> 
            size_t append_utf_str_impl(
               const typename StringEncoding::code_unit_type* str,
               StringEncoding encoding,
               typename boost::enable_if<
               boost::is_same<typename StringEncoding::code_unit_type, typename EncodingT::code_unit_type>
               >::type* = 0 )
            {
                size_t count = 0;
                while (*str != 0) {
                    append(*++str);
                    ++count;
                }
                return count;
            }
            template <typename StringEncoding> 
            size_t append_utf_str_impl(
               const typename StringEncoding::code_unit_type* str, std::size_t len,
               StringEncoding encoding,
               typename boost::enable_if<
               boost::is_same<typename StringEncoding::code_unit_type, typename EncodingT::code_unit_type>
               >::type* = 0 )
            {
                append(str, len);
                return len;
            }
            
            
            // Append zero-terminated UTF-8 string to UTF-16/UTF-32 buffer
            template <typename StringEncoding> 
            size_t append_utf_str_impl(
               const typename StringEncoding::code_unit_type* str,
               StringEncoding encoding,
               typename boost::enable_if<
                    boost::mpl::and_< 
                        boost::is_same<UTF_8_encoding_tag, StringEncoding>,
                        boost::mpl::or_<
                            boost::is_base_of<UTF_16_encoding_tag, EncodingT>,
                            boost::is_base_of<UTF_32_encoding_tag, EncodingT>
                        >
                    >
               >::type* = 0 )
            {
                typedef typename StringEncoding::code_unit_type  code_unit_t;  
                
                const code_unit_t* first = str;
                size_t count = 0;
                while (*first) {
                    reserve(size() + 4); // grow for max code units that can be generated
                    int result = unicode::convert_one_unsafe(first, first + 4, StringEncoding(), p_, buffer_end_, EncodingT());
                    if (result > 0) {
                        count += result;
                    }
                    else {
                        break;  // ERROR:
                    }
                    
                    assert(p_ <= buffer_end_);
                }
                return count;
            }
            
            // Append UTF-8 string with length len to UTF-16/UTF-32
            template <typename StringEncoding> 
            size_t append_utf_str_impl(
                   const typename StringEncoding::code_unit_type* str,
                   std::size_t len,
                   StringEncoding encoding,
                   typename boost::enable_if<
                        boost::mpl::and_< 
                            boost::is_same<UTF_8_encoding_tag, StringEncoding>,
                            boost::mpl::or_<
                                boost::is_base_of<UTF_16_encoding_tag, EncodingT>,
                                boost::is_base_of<UTF_32_encoding_tag, EncodingT>
                            >
                        >
                   >::type* = 0 )
            {
                typedef typename StringEncoding::code_unit_type  code_unit_t;  
                
                const code_unit_t* first = str;
                const code_unit_t* last = str + len;
                size_t count = 0;
                reserve(len); // reasonable grow, like reserve.
                while (first < last) {
                    reserve(size() + 4); // if required, grow for max code units that can be generated
                    int result = unicode::convert_one_unsafe(first, last, StringEncoding(), 
                                                         p_, EncodingT());
                    if (result > 0) {
                        count += result;
                    }
                    else {
                        break;  // ERROR:
                    }
                    
                    assert(first <= last);
                    assert(p_ <= buffer_end_);
                }
                return count;
            }
            
            
            
            bool grow(size_type min_size) 
            {
                if (min_size <= capazity())
                    return true;
                size_type new_size = auto_buffer_size_;
                while (new_size < min_size)
                    new_size = 2 * new_size;
                
                size_type current_size = size();
                buffer_type old_buffer_start_ = buffer_start_;
                bool allocationSuccess = false;
                
                while (not allocationSuccess) {
                    if (buffer_start_ == auto_buffer_) {
                        buffer_start_ = (buffer_type)malloc(new_size * sizeof(code_unit_t));
                        if (buffer_start_) {
                            std::copy(auto_buffer_, auto_buffer_ + auto_buffer_size_, buffer_start_);
                            //memcpy(buffer_start_, auto_buffer_, sizeof(code_unit_t) * auto_buffer_size_);
                        }
                    } else {
                        buffer_start_ = (buffer_type)realloc(buffer_start_, new_size * sizeof(code_unit_t));
                    }
                    if (buffer_start_) {
                        buffer_end_ = buffer_start_ + new_size;
                        p_ = buffer_start_ + current_size;
                        allocationSuccess = true;
                    } 
                    else {
                        // allocation failed
                        buffer_start_ = old_buffer_start_;
                        // reduce the new size a bit:
                        if (new_size > auto_buffer_size_) {
                            new_size -= std::min(size_type(auto_buffer_size_), new_size - auto_buffer_size_);
                        }
                        if (new_size < min_size) {
                            // give up
                            break;
                        }
                    }
                }    
                return allocationSuccess;
            }
        };
        
        template <typename EncodingT, int AutoBufferSize = 1024> 
        class string_buffer : public string_buffer_base<EncodingT> {
        public:
            typedef string_buffer_base<EncodingT>       base_type;
            typedef typename base_type::buffer_type     buffer_type;
            typedef typename base_type::code_unit_t     code_unit_t;
//            typedef typename base_type::string_type     string_type;
            typedef typename base_type::size_type       size_type;
            typedef typename base_type::from_endian_t   from_endian_t;            
            typedef typename base_type::to_endian_t     to_endian_t;
            
        private:
            code_unit_t         buffer_[AutoBufferSize];
        public:
            string_buffer() 
            : string_buffer_base<EncodingT>(buffer_, AutoBufferSize)
            {
            } 
        };
        

    }  // namespace internal

}  // namespace json


#endif // JSON_STRING_BUFFER_HPP
