#include <cnoid/URDFBodyWriter>
#include <cnoid/BodyLoader>
#include <cnoid/Body>

#if 0
#include <boost/program_options.hpp>
#else
#include <CLI11.hpp>
#endif

#include <iostream>

using namespace cnoid;
#if 0
using namespace boost::program_options;
#endif

int main(int argc, char **argv)
{
    std::string fname;
    std::string outputf;
    std::string rname;
    bool no_offset = false;
    bool no_geom   = false;
    bool no_xacro  = false;
    bool verbose   = false;
    bool export_devices = false;
    bool use_primitive = true;
    std::string fp_;
    std::string up_;
#if 0
    options_description desc("$ body2urdf [options] inputfile.body");
    desc.add_options()
    ("robot-name,r", value<std::string>(), "robot name")
    ("output,O", value<std::string>(), "output file")
    ("mesh-file-prefix", value<std::string>()->default_value("/tmp/"), "prefix of mesh file (e.g. directory)")
    ("mesh-url-prefix", value<std::string>()->default_value("file:///tmp/"), "prefix of url refered in output URDF file")
    ("no-offset", "surpress output about link position ")
    ("no-geometry", "suppress output about geometry-info")
    ("no-xacro", "do not use xacro")
    ("no-primitive", "do not use primitive type of geometry")
    ("export-devices", "export devices as link")
    ("verbose,v", "verbose message")
    ("help,h", "help")
    ;

    variables_map vm;
    auto const presult = parse_command_line(argc, argv, desc);
    store(presult, vm);
    notify(vm);

    if (vm.count("help") > 0) {
        std::cout << desc << std::endl;
        return 0;
    }

    for (auto const& str : collect_unrecognized(presult.options, include_positional)) {
        fname = str;
    }

    if (vm.count("output") > 0) {
        outputf = vm["option"].as<std::string>();
    }
    if (vm.count("robot-name") > 0) {
        rname = vm["robot-name"].as<std::string>();
    }
    if (vm.count("no-offset") > 0) {
        no_offset = true;
    }
    if (vm.count("no-geometry") > 0) {
        no_geom = true;
    }
    if (vm.count("no-xacro") > 0) {
        no_xacro = true;
    }
    if (vm.count("no-primitive") > 0) {
        use_primitive = false;
    }
    if (vm.count("verbose") > 0) {
        verbose = true;
    }
    if (vm.count("export-devices") > 0) {
        export_devices = true;
    }

    fp_ = vm["mesh-file-prefix"].as<std::string>();
    up_ = vm["mesh-url-prefix"].as<std::string>();
#else
    CLI::App vm{"Converting .body to .urdf"};
    bool no_primitive{false};
    vm.add_option("-r,--robot-name", rname, "robot name");
    vm.add_option("-O,--output", outputf, "output file");
    vm.add_option("--mesh-file-prefix", fp_, "prefix of mesh file (e.g. directory)");
    vm.add_option("--mesh-url-prefix", up_, "prefix of url refered in output URDF file");
    vm.add_flag("--no-offset", no_offset, "surpress output about link position ");
    vm.add_flag("--no-geometry", no_geom, "suppress output about geometry-info");
    vm.add_flag("--no-xacro", no_xacro, "do not use xacro");
    vm.add_flag("--no-primitive", no_primitive, "do not use primitive type of geometry");
    vm.add_flag("--export-devices", export_devices, "export devices as link");
    vm.add_flag("-v,--verbose", verbose, "verbose message");

    vm.add_option("in_file_name", fname, "name of input file(.body)");
    CLI11_PARSE(vm, argc, argv);

    use_primitive = !no_primitive;
#endif
    /////
    if(fname.empty()) {
        std::cerr << "Error! / please input filename" << std::endl;
        return -1;
    }
    if (verbose) {
        std::cerr << "in_file_name: " << fname << std::endl;
        std::cerr << "robot name:     " << rname << std::endl;
        std::cerr << "no-offset:      "   << no_offset << std::endl;
        std::cerr << "no-geometry:    " << no_geom << std::endl;
        std::cerr << "no-xacro:       "    << no_xacro << std::endl;
        std::cerr << "export-devices: "    << export_devices << std::endl;
        std::cerr << "output: " << outputf << std::endl;
    }
    BodyLoader bl;
    if (verbose) {
        bl.setMessageSink(std::cerr);
        bl.setVerbose(true);
    }

    Body *bd = bl.load(fname);
    if (!bd) {
        std::cerr << "body open error! : " << fname << std::endl;
        return -1;
    }

    bd->updateLinkTree();
    bd->initializePosition();
    bd->calcForwardKinematics();

    URDFBodyWriter ubw;
    if (verbose) {
        ubw.setMessageSink(std::cerr);
        ubw.setVerbose(true);
    }
    ubw.setUseXacro(!no_xacro);
    ubw.setAddGeometry(!no_geom);
    ubw.setAddOffset(!no_offset);
    ubw.setExportDevices(export_devices);
    ubw.setUseURDFPrimitiveGeometry(use_primitive);

    ubw.setMeshFilePrefix(fp_);
    ubw.setMeshURLPrefix(up_);
    if(!rname.empty()){
        ubw.setRobotName(rname);
    }

    if (outputf.empty()) {
        std::string out;
        ubw.writeBodyAsString(bd, out);
        std::cout << out << std::endl;
    } else {
        bool res = ubw.writeBody(bd, outputf);
        if(!res) {
            std::cerr << "failed to output : " << outputf << std::endl;
            return -1;
        }
    }

    return 0;
}
