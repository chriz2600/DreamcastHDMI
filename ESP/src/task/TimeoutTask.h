#ifndef TIMEOUT_TASK_H
#define TIMEOUT_TASK_H

#include "../global.h"
#include <Task.h>

extern TaskManager taskManager;

typedef std::function<void(uint32_t timedone, bool done)> TimeoutCallbackHandler;

class TimeoutTask : public Task {

    public:
        TimeoutTask(uint8_t v) :
            Task(v)
        { };

        void setTimeout(uint32_t _timeout) {
            timeout = _timeout;
        }

        void setTimeoutCallback(TimeoutCallbackHandler _handler) {
            handler = _handler;
        }

    private:
        uint32_t timeout;
        TimeoutCallbackHandler handler;

        virtual bool OnStart() {
            DEBUG("TimeoutTask: OnStart\n");
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            deltaTime = TaskTimeToMs(deltaTime);

            if (deltaTime > timeout) {
                timeout = 0;
            } else {
                timeout -= deltaTime;
            }

            handler(timeout, (timeout == 0));
        }

        virtual void OnStop() {
            DEBUG("TimeoutTask: OnStop\n");
        }
};

#endif