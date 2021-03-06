#pragma once

#include "io/archive.h"
#include "task_mem_vision.h"

namespace nano
{
        ///
        /// CIFAR100 task:
        ///      - object classification
        ///      - 32x32 color images as inputs
        ///      - 100 outputs (100 labels)
        ///
        /// http://www.cs.toronto.edu/~kriz/cifar.html
        ///
        class cifar100_task_t final : public mem_vision_task_t
        {
        public:

                explicit cifar100_task_t(const string_t& configuration = string_t());

        private:

                virtual bool populate() override;

                // load binary file
                bool load_binary(const string_t& filename, istream_t&, const protocol, const size_t);
        };
}

