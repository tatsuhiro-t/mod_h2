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

#include <assert.h>

#include <httpd.h>
#include <http_core.h>
#include <http_log.h>

#include "h2_private.h"
#include "h2_task.h"
#include "h2_worker.h"

struct h2_worker {
    int id;
    apr_thread_t *thread;
    apr_pool_t *pool;
    h2_worker_task_next_fn *get_next;
    h2_worker_task_done_fn *task_done;
    h2_worker_done_fn *worker_done;
    void *ctx;
    int aborted;
    
    struct h2_task *current;
};

static void *execute(apr_thread_t *thread, void *wctx)
{
    h2_worker *worker = (h2_worker *)wctx;
    apr_status_t status = APR_SUCCESS;
    worker->current = NULL;
    
    while (!worker->aborted) {
        if (worker->current) {
            apr_status_t status = h2_task_do(worker->current);
            worker->current = worker->task_done(worker, worker->current,
                                                status, worker->ctx);
        }
        if (!worker->current) {
            status = worker->get_next(worker, &worker->current,worker->ctx);
        }
    }
    
    worker->worker_done(worker, worker->ctx);
    apr_thread_exit(thread, status);
    return NULL;
}

h2_worker *h2_worker_create(int id,
                            apr_pool_t *pool,
                            apr_threadattr_t *attr,
                            h2_worker_task_next_fn *get_next,
                            h2_worker_task_done_fn *task_done,
                            h2_worker_done_fn *worker_done,
                            void *ctx)
{
    h2_worker *w = apr_pcalloc(pool, sizeof(h2_worker));
    if (w) {
        w->id = id;
        w->pool = pool;
        w->get_next = get_next;
        w->task_done = task_done;
        w->worker_done = worker_done;
        w->ctx = ctx;
        
        apr_thread_create(&w->thread, attr, execute, w, pool);
    }
    return w;
}

apr_status_t h2_worker_destroy(h2_worker *worker)
{
    return APR_SUCCESS;
}

int h2_worker_get_id(h2_worker *worker)
{
    return worker->id;
}

void h2_worker_abort(h2_worker *worker)
{
    worker->aborted = 1;
}

int h2_worker_is_aborted(h2_worker *worker)
{
    return worker->aborted;
}

