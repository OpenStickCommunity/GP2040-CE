#ifndef _GPSCREEN_H_
#define _GPSCREEN_H_

#include <vector>
#include <cstring>
#include <memory>
#include "GPWidget.h"

class GPScreen : public GPWidget {
    public:
        void draw();
        std::string header = "";
        std::string footer = "";
        virtual int8_t update();
        void clear();

        template<typename _GPWidget, typename... _GPArgs>
        _GPWidget* addElement(_GPArgs&&... args) {
            using GPW = std::unique_ptr<_GPWidget>;
            GPW new_element = std::make_unique<_GPWidget>(std::forward<_GPArgs>(args)...);
            GPWidget* raw_ptr = new_element.get();
            displayList.emplace_back(std::move(new_element));
            return static_cast<_GPWidget*>(raw_ptr);
        }
    protected:
        virtual void drawScreen() = 0;

    private:
        std::vector<std::unique_ptr<GPWidget>> displayList;
};

#endif