//
//  JPAsyncJsonParser.h
//
//  Created by Andreas Grosam on 7/8/11.
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

#ifndef JSON_OBJC_JP_ASYNC_JSON_PARSER_H
#define JSON_OBJC_JP_ASYNC_JSON_PARSER_H  


#import <Foundation/Foundation.h>
#import "JPJsonCommon.h"   // for JPJsonParserOptions


/** 
 While a `JPJsonParser` can only parse input which contains one or more JSON 
 documents that is provided in _one_ buffer which is a contiguous sequence of
 bytes in memory, a `JPAsyncJsonParser` is capable to parse input which is 
 partitioned into separate data buffers. The concatenated content may comprise 
 one or more JSON documents.
 
 When downloading a resource from the net, receiveing partitioned JSON content 
 separated into several buffers will be the usual case. `JPAsyncJsonParser` 
 provides an interface where a sequence of NSData buffers can be sent via the 
 message `parseBuffer:` to a `JPAsyncJsonParser` instance. The sequence of data 
 buffers make up the complete input which may contain one or more JSON documents. 
 This makes `JPAsyncJsonParser` especially suited to receive partial input which
 will be received as a sequence of data buffers, e.g. from a `NSURLConnection` 
 delegate or through a `NSStream` interface.
 
 A `JPAsyncJsonParser` instance will invoke the underlaying parser
 asynchronously. That is, when an `JPAsyncJsonParser` instance receives the 
 `start` message, the underlaying json parser will be executed on a different 
 thread and the method `start` returns  immediately. The underlaying json parser 
 will then wait for incomming data buffer to become available for parsing.
 
 The client is responsible to provide the sequence of `NSData` objects which 
 will be sent by message `parserBuffer:` to the `JPAsyncJsonParser` instance.
 The underlaying json parser in turn will consume this data buffer and start 
 parsing it. As a result it will send parse events to the semantic actions 
 object as long as there is data available, or until the parser receives an 
 `EOF` from the input. If there is no data available and the parser expects 
 yet more input, the parser's thread will be blocked until data is available 
 again.
 
 A client of an asynchronous parser will be notified about the result of a 
 semantic actions object and other events through the means of _handler blocks_ 
 which will be assigend the semantic actions object via properties. The exact 
 behavior depends on the concrete semantic actions class. For more information
 on semantic actions see <JPSemanticActionsProtocol>, <JPSemanticActionsBase>,
 <JPSemanticActions> and <JPStreamSemanticActions>.
 
   
 
 A `JPAsyncJsonParser` object can only be used once for parsing the input. That is,
 once the message `start` has been sent, it cannot be invoked again. In order to 
 parse another input, a new parser object must be created. The input can consist
 of many JSON documents however, and the total length of the input stream can be 
 virtually infinitive.
*/ 
 

/*
 The `JPAsyncJsonParser` will be configured through its "semantic actions" 
 object. Each parser requires this object - since it is this object that defines 
 what to do with the pieces of strings, numbers and other values which will be 
 generated by the parser when scanning the JSON text.
 
 There can be many kinds of semantic actions. For instance, JPSemanticActions
 is the "default" action which should be used when you want to create a 
 representation of the JSON text which is a container, that is, a hierarchy of 
 objects, namely of NSDictionaries, NSArrays, and NSNumbers and NSStrings. 
 
 There can be other semantic actions, which purpose may be to just validate a
 JSON text, or which can be used to directly bind to Core Data objects. The
 latter may lower the execution costs when you want to safe the data which you
 receive from a JSON text into Core Data objects anyway. So, you could achieve 
 this, through a special "binder semantic actions object" - which directly 
 interfaces to Core Data objects - instead to create an intermediate JSON data
 structure first, and then use this JSON data structure to set the Core Data
 objects.
 
 
 TODO: implement method clear which resets the JPAsyncJsonParser object which
 in turn can be used to invoke start again.
*/



//typedef void (^JPAsyncJsonParser_CompletionBlockType)(void);
//typedef void (^JPAsyncJsonParser_DataHandlerBlockType)(id);
//typedef void (^JPAsyncJsonParser_ErrorHandlerBlockType)(NSError*);


@class JPSemanticActionsBase;

@interface JPAsyncJsonParser : NSObject 



/** @name Intialization */


/**
 *Designated Initializer*

 If object `semanticActions` equals nil an instance of class `JPSemanticActions`
 will be created and initialized with default properties. The default values 
 which will be set are documented in class <JPSemanticActions>.

 If parameter `workerQueue` equals `NULL` the parsing routines will be scheduled 
 on the global dispatch queue. This is usually a good choice.

 The semantic actions handler dispatch queue and the parser's worker dispatch 
 queue must not be the same, unless the semantic actions object property 
 `parseMultipleDocumentsAsynchronously` equals `YES`.
 
 @param semanticActions A semantic actions object or `nil`.
 
 @param workerQueue A dispatch queue or `NULL`.
*/
- (id) initWithSemanticActions:(JPSemanticActionsBase*)semanticActions
            workerDispatchQueue:(dispatch_queue_t)workerQueue;


/**
 Invokes the designated initializer with parameter `semanticActions` set to `nil`
 and `workerQueue` set to `NULL`.
 
 @see -initWithSemanticActions:workerDispatchQueue:
*/ 
- (id) init;


/** @name Properties */

@property (nonatomic, readonly) size_t              bufferQueueSize;
@property (nonatomic, assign)   size_t              bufferQueueCapacity;



/**
 Returns the semantic actions object.
*/ 
@property (nonatomic, readonly, retain) JPSemanticActionsBase* semanticActions;


/** @name Starting the Parser Asynchronously */

/**
 Start the parser asynchronously. The parser will wait for data buffers to
 become available provided by method parseBuffer:. Once the first buffer is 
 available, the parser will try to determine the encoding of the input. If
 any error occures while detecting the encoding, the parser will issue an 
 error which will cause the error handler to be called (if it is not NULL).
 Otherwise, the parser will continue to parse the input from the buffers, 
 until no more buffers are available.

 Depending on the actual semantic actions object, their handlers may be 
 called whenever the start of a JSON document was found in the innput stream 
 and whenever a JSON document has been created. Finally, if the end of the 
 data is detected a completion handler may be called.
 
 @return Returns YES if the parser has not yet been started previously.
*/
- (BOOL) start;


/** @name Providing Data Buffers */

/**
 Push a data buffer to the parser's internal buffer queue which the parser 
 will start to process when it becomes ready. 
 
 The content of buffer may contain partial JSON text in which case subsequent 
 invokations of `parseBuffer:` will be required in order to finish parsing of 
 one or more JSON texts.
 
 The method will block until after the parser is ready to consume this buffer,
 that is, when it has finished the previous buffer or when it is idle. It does 
 not block however for the time it takes to process this buffer.
 
 buffer will be retained for the duration of use and then released again. 

 @param buffer A `NSData` object possibly containing a partial input. 
 If buffer equals `nil` the parser treats it as `EOF` and stops parsing.
 
 @return Returns YES if the parser will consume this buffer. Returns NO if a 
 timeout occurred, or if the parser is in cancel or error state.
 
 @warning *Caution:* The buffer's byte sequence may start or end only at
 complete Unicode code unites. That is, for instance, UTF-8 encoded text may
 start and end at any byte boundary, while UTF-16 and UTF-32 requires to start 
 and end at their respective code units (two bytes and four bytes).
*/ 
- (BOOL) parseBuffer:(NSData*)buffer;


/** @name Canceling an Asynchronously Running Parser */

/**
 Cancels the parser which forces it to exit as soon as possible.
 After canceling the parser's result and error state is undefined.
*/ 
- (void) cancel;


/** @name Check if a Parser is Running */

/**
 Returns YES if the parser has been started and is currently running.
 */
- (BOOL) isRunning;



@end

#endif
