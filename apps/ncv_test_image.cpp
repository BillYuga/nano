#include "nanocv/string.h"
#include "nanocv/measure.hpp"
#include "nanocv/vision/image.h"
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
        using namespace ncv;
        
        // parse the command line
        boost::program_options::options_description po_desc("", 160);
        po_desc.add_options()("help,h", "test program");
        po_desc.add_options()("input,i",
                boost::program_options::value<ncv::string_t>(),
                "input image path");
        po_desc.add_options()("scale,s",
                boost::program_options::value<ncv::scalar_t>()->default_value(1.0),
                "scaling factor [0.1, 10.0]");
        po_desc.add_options()("translate,t",
                boost::program_options::value<ncv::coord_t>()->default_value(0),
                "translate range in pixels [0, 256]");
        po_desc.add_options()("noise-offset",
                boost::program_options::value<ncv::scalar_t>()->default_value(0.0),
                "noise offset [-100, +100.0]");
        po_desc.add_options()("noise-range",
                boost::program_options::value<ncv::scalar_t>()->default_value(0.0),
                "noise range [0.0, 100.0]");
        po_desc.add_options()("noise-sigma",
                boost::program_options::value<ncv::scalar_t>()->default_value(0.0),
                "noise gaussian bluring standard deviation [0.0, 10.0]");
        po_desc.add_options()("noise-channel",
                boost::program_options::value<ncv::string_t>()->default_value("rgba"),
                "noise color channel [luma|red|green|blue|rgba]");
        po_desc.add_options()("gauss-sigma",
                boost::program_options::value<ncv::scalar_t>()->default_value(0.0),
                "gaussian bluring standard deviation [0.0, 10.0]");
        po_desc.add_options()("gauss-channel",
                boost::program_options::value<ncv::string_t>()->default_value("rgba"),
                "gaussian bluring color channel [luma|red|green|blue|rgba]");
        po_desc.add_options()("output,o",
                boost::program_options::value<ncv::string_t>(),
                "output image path");
        po_desc.add_options()("luma",
                "load and process the image as luma (grayscale)");
        po_desc.add_options()("rgba",
                "load and process the image as RGBA (color)");
	
        boost::program_options::variables_map po_vm;
        boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv).options(po_desc).run(),
                po_vm);
        boost::program_options::notify(po_vm);
        		
        // check arguments and options
        if (	po_vm.empty() ||
                !po_vm.count("input") ||
                !po_vm.count("output") ||
                po_vm.count("help"))
        {
                std::cout << po_desc;
                return EXIT_FAILURE;
        }

        const string_t cmd_input = po_vm["input"].as<string_t>();

        const string_t cmd_output = po_vm["output"].as<string_t>();
        const bool cmd_luma = po_vm.count("luma");
        const bool cmd_rgba = po_vm.count("rgba");        

        if (    (!cmd_luma && !cmd_rgba) ||
                (cmd_luma && cmd_rgba))
        {
                std::cout << po_desc;
                return EXIT_FAILURE;
        }

        // load input image
        image_t image;
        ncv::measure_critical_and_log(
               [&] () { return (cmd_luma ? image.load_luma(cmd_input) : image.load_rgba(cmd_input)); },
               "loaded image from <" + cmd_input + ">",
               "failed to load image from <" + cmd_input + ">");

        log_info () << "image: " << image.cols() << "x" << image.rows() << " pixels, "
                    << (image.is_luma() ? "[luma]" : "[rgba]") << ".";

        log_info () << "image: " << image.cols() << "x" << image.rows() << " pixels, "
                    << (image.is_luma() ? "[luma]" : "[rgba]") << ".";

        // save output image
        ncv::measure_critical_and_log(
                [&] () { return image.save(cmd_output); },
                "saved image to <" + cmd_output + ">",
                "failed to save to <" + cmd_output + ">");
		
        // OK
        log_info() << ncv::done;
        return EXIT_SUCCESS;
}
