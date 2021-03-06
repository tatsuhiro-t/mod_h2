/* Copyright 2015 greenbytes GmbH (https://www.greenbytes.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __mod_h2__h2_response__
#define __mod_h2__h2_response__

/* h2_response is just the data belonging the the head of a HTTP response,
 * suitable prepared to be fed to nghttp2 for response submit. 
 */

struct h2_bucket;

typedef struct h2_response {
    int stream_id;
    apr_status_t task_status;
    const char *http_status;
    struct h2_bucket *data;

    long content_length;
    int chunked;
    
    apr_size_t nvlen;
    const nghttp2_nv nv;
    /* must be last element */
} h2_response;

h2_response *h2_response_create(int stream_id,
                                  apr_status_t task_status,
                                  const char *http_status,
                                  apr_array_header_t *hlines,
                                  struct h2_bucket *data,
                                  apr_pool_t *pool);

void h2_response_destroy(h2_response *head);

long h2_response_get_content_length(h2_response *resp);

#endif /* defined(__mod_h2__h2_response__) */
