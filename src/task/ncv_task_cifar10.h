#ifndef NANOCV_TASK_CIFAR10_H
#define NANOCV_TASK_CIFAR10_H

#include "ncv_task_class.h"

namespace ncv
{
//	////////////////////////////////////////////////////////////////////////////////
//        // CIFAR10 task:
//        //      - object classification
//        //      - 32x32 inputs
//        //      - 10 outputs (10 labels)
//	////////////////////////////////////////////////////////////////////////////////
	
//        class cifar10_task_t : public class_task_t
//	{
//	public:
                
//                // Constructor
//                cifar10_task_t();
                
//                // Clone the object
//                virtual rtask_t clone() const { return rtask_t(new cifar10_task_t(*this)); }
                
//                // Load data
//                virtual bool load(const string_t& dir);
                                                   
//	private:
                                                   
//                // Read binary file into either <dtype1> or <dtype2> datasets with the given probability
//                bool load(const string_t& basename, const string_t& bfile,
//                        data_enum dtype1, data_enum dtype2, scalar_t prob);
//	};
}

#endif // NANOCV_TASK_CIFAR10_H
