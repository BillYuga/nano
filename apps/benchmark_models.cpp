#include "nano.h"
#include "class.h"
#include "measure.h"
#include "text/table.h"
#include "accumulator.h"
#include "text/cmdline.h"
#include "math/random.h"
#include "tensor/numeric.h"
#include "measure_and_log.h"
#include "layers/make_layers.h"
#include "tasks/task_charset.h"
#include "text/table_row_mark.h"
#include <iostream>

int main(int argc, const char *argv[])
{
        using namespace nano;

        // parse the command line
        nano::cmdline_t cmdline("benchmark models");
        cmdline.add("s", "samples",     "number of samples to use [100, 100000]", "10000");
        cmdline.add("c", "conn",        "plane connectivity for convolution networks [1, 16]", "8");
        cmdline.add("", "mlps",         "benchmark MLP models");
        cmdline.add("", "convnets",     "benchmark convolution networks");
        cmdline.add("", "forward",      "evaluate the \'forward\' pass (output)");
        cmdline.add("", "backward",     "evaluate the \'backward' pass (gradient)");
        cmdline.add("", "activation",   "activation layer (act-unit, act-tanh, act-splus, act-snorm)", "act-snorm");
        cmdline.add("", "detailed",     "print detailed measurements (e.g. per-layer)");

        cmdline.process(argc, argv);

        // check arguments and options
        const auto cmd_samples = nano::clamp(cmdline.get<size_t>("samples"), 100, 100 * 1000);
        const auto conn = nano::clamp(cmdline.get<int>("conn"), 1, 16);
        const auto cmd_forward = cmdline.has("forward");
        const auto cmd_backward = cmdline.has("backward");
        const auto cmd_mlps = cmdline.has("mlps");
        const auto cmd_convnets = cmdline.has("convnets");
        const auto activation = cmdline.get("activation");
        const auto cmd_detailed = cmdline.has("detailed");

        if (!cmd_forward && !cmd_backward)
        {
                cmdline.usage();
        }

        if (!cmd_mlps && !cmd_convnets)
        {
                cmdline.usage();
        }

        const tensor_size_t cmd_rows = 28;
        const tensor_size_t cmd_cols = 28;
        const color_mode cmd_color = color_mode::luma;

        const size_t cmd_min_nthreads = 1;
        const size_t cmd_max_nthreads = nano::logical_cpus();

        // generate synthetic task
        charset_task_t task(to_params(
                "type", charset_mode::digit, "color", cmd_color, "irows", cmd_rows, "icols", cmd_cols, "count", cmd_samples));
        task.load();

        // construct models
        const string_t mlp0;
        const string_t mlp1 = mlp0 + make_affine_layer(128, activation);
        const string_t mlp2 = mlp1 + make_affine_layer(128, activation);
        const string_t mlp3 = mlp2 + make_affine_layer(128, activation);
        const string_t mlp4 = mlp3 + make_affine_layer(128, activation);
        const string_t mlp5 = mlp4 + make_affine_layer(128, activation);

        const string_t convnet0;
        const string_t convnet1 = convnet0 + make_conv_layer(64, 7, 7, 1, activation);
        const string_t convnet2 = convnet1 + make_conv_layer(64, 7, 7, conn, activation);
        const string_t convnet3 = convnet2 + make_conv_layer(64, 5, 5, conn, activation);
        const string_t convnet4 = convnet3 + make_conv_layer(64, 5, 5, conn, activation);
        const string_t convnet5 = convnet4 + make_conv_layer(64, 3, 3, conn, activation);

        const string_t outlayer = make_output_layer(task.odims());

        std::vector<std::pair<string_t, string_t>> networks;
        #define DEFINE(config) networks.emplace_back(config + outlayer, NANO_STRINGIFY(config))
        if (cmd_mlps)
        {
                DEFINE(mlp0);
                DEFINE(mlp1);
                DEFINE(mlp2);
                DEFINE(mlp3);
                DEFINE(mlp4);
                DEFINE(mlp5);
        }
        if (cmd_convnets)
        {
                DEFINE(convnet1);
                DEFINE(convnet2);
                DEFINE(convnet3);
                DEFINE(convnet4);
                DEFINE(convnet5);
        }
        #undef DEFINE

        const auto loss = nano::get_losses().get("logistic");
        const auto criterion = nano::get_criteria().get("avg");

        // construct tables to compare models
        nano::table_t ftable("model-forward [us] / sample");
        nano::table_t btable("model-backward [us] / sample");

        for (size_t nthreads = cmd_min_nthreads; nthreads <= cmd_max_nthreads; ++ nthreads)
        {
                ftable.header() << (nano::to_string(nthreads) + "xCPU");
                btable.header() << (nano::to_string(nthreads) + "xCPU");
        }

        // evaluate models
        for (const auto& config : networks)
        {
                const string_t cmd_network = config.first;
                const string_t cmd_name = config.second;

                log_info() << "<<< running network [" << cmd_network << "] ...";

                // create feed-forward network
                const auto model = nano::get_models().get("forward-network", cmd_network);
                model->resize(task);
                model->random_params();
                model->describe();

                auto& frow = ftable.append(cmd_name + " (" + nano::to_string(model->psize()) + ")");
                auto& brow = btable.append(cmd_name + " (" + nano::to_string(model->psize()) + ")");

                const auto fold = fold_t{0, protocol::train};

                std::vector<model_t::timings_t> ftimings(cmd_max_nthreads + 1);
                std::vector<model_t::timings_t> btimings(cmd_max_nthreads + 1);

                // process the samples
                for (size_t nthreads = cmd_min_nthreads; nthreads <= cmd_max_nthreads; ++ nthreads)
                {
                        accumulator_t acc(*model, *loss, *criterion);
                        acc.lambda(scalar_t(0.1));
                        acc.threads(nthreads);

                        if (cmd_forward)
                        {
                                const auto duration = nano::measure_robustly_usec([&] ()
                                {
                                        acc.mode(criterion_t::type::value);
                                        acc.update(task, fold);
                                }, 1);

                                log_info() << "<<< processed [" << acc.count()
                                           << "] forward samples in " << duration.count() << " us.";

                                frow << idiv(static_cast<size_t>(duration.count()), acc.count());

                                ftimings[nthreads] = acc.timings();
                        }

                        if (cmd_backward)
                        {
                                const auto duration = nano::measure_robustly_usec([&] ()
                                {
                                        acc.mode(criterion_t::type::vgrad);
                                        acc.update(task, fold);
                                }, 1);

                                log_info() << "<<< processed [" << acc.count()
                                           << "] backward samples in " << duration.count() << " us.";

                                brow << idiv(static_cast<size_t>(duration.count()), acc.count());

                                btimings[nthreads] = acc.timings();
                        }
                }

                // detailed per-component (e.g. per-layer) timing information
                const auto print_timings = [&] (table_t& table, const string_t& basename,
                        const std::vector<model_t::timings_t>& timings)
                {
                        for (const auto& timing0 : timings[cmd_min_nthreads])
                        {
                                auto& row = table.append(basename + timing0.first);

                                for (size_t nthreads = cmd_min_nthreads; nthreads <= cmd_max_nthreads; ++ nthreads)
                                {
                                        const auto& timingT = timings[nthreads];
                                        assert(timingT.find(timing0.first) != timingT.end());
                                        row << timingT.find(timing0.first)->second.avg();
                                }
                        }
                };

                if (cmd_forward && cmd_detailed)
                {
                        print_timings(ftable, ">", ftimings);
                }

                if (cmd_backward && cmd_detailed)
                {
                        print_timings(btable, ">", btimings);
                }
        }

        // print results
        if (cmd_forward)
        {
                ftable.mark(nano::make_table_mark_minimum_percentage_cols<size_t>(5));
                std::cout << ftable;
        }
        if (cmd_backward)
        {
                btable.mark(nano::make_table_mark_minimum_percentage_cols<size_t>(5));
                std::cout << btable;
        }

        // OK
        log_info() << done;
        return EXIT_SUCCESS;
}
