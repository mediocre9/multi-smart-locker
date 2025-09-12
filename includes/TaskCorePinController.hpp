#ifndef TASK_CORE_PIN_CONTROLLER_H
#define TASK_CORE_PIN_CONTROLLER_H
#include <cstdint>

class TaskCorePinController final {
public:
    TaskCorePinController() = default;

    TaskCorePinController& pinTaskToCPUCore(void (*taskHandler)(void*)) {
        if (_task != nullptr) {
            vTaskDelete(_task);
            _task = nullptr;
        }

        xTaskCreatePinnedToCore(
            taskHandler,
            "task",
            _taskStackSize,
            nullptr,
            _taskPriorityLevel,
            &_task,
            _coreId);

        return *this;
    }

    TaskCorePinController& setTaskStackSize(std::uint16_t stackSize) {
        _taskStackSize = stackSize;
        return *this;
    }

    TaskCorePinController& setPriorityLevel(std::uint16_t level) {
        _taskPriorityLevel = level;
        return *this;
    }

    TaskCorePinController& setCoreId(std::uint16_t core) {
        _coreId = core;
        return *this;
    }

private:
    TaskHandle_t _task{nullptr};
    std::uint16_t _coreId{1};
    std::uint16_t _taskPriorityLevel{1};
    std::uint16_t _taskStackSize{10 * 1000};  // default 10kb task stack size.....
};

#endif