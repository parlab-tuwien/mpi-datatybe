#! /usr/bin/env python

import os
import sys
import math
import datetime
import subprocess
import shutil
import argparse

base_path = os.path.dirname( os.path.realpath( sys.argv[0] ) )
sys.path.append(base_path) 

SOURCE_DIR = os.path.join(base_path, "src")
DEFAULT_CONFIG_DIR = os.path.join(base_path, "config")
DEFAULT_CONFIG_FILE = os.path.join(DEFAULT_CONFIG_DIR, "build.conf")

COMMENT_STR = "#" * 50

def run_command(command_as_list, target_dir, print_output = True):
    try:
        routput = subprocess.check_output(command_as_list,
                                        cwd = target_dir,
                                        stderr=subprocess.STDOUT
                                        )
        if print_output:
            print routput
    except subprocess.CalledProcessError as e:
        sys.exit(e.output)
           
        
def cmake_source_code(source_code_dir, compile_options_str, build_dir = None):    
    cache_file = os.path.join(source_code_dir, "CMakeCache.txt")
    if os.path.exists(cache_file):
        print "Removing %s" % (cache_file)
        os.remove(cache_file)
    cmakefiles = os.path.join(source_code_dir, "CMakeFiles")
    if os.path.exists(cmakefiles):
        print "Removing %s" % (cmakefiles)
        shutil.rmtree(cmakefiles)
    
    if build_dir == None:
        build_dir = source_code_dir
    
    command = ["cmake"]
    command = command + compile_options_str.split(" ")
    command = command + [ source_code_dir ]

    run_command(command, build_dir)
    
    
def compile_source_code(build_dir, install = False):
    command = ["make" ]
    command = command + [ "-j", "4"]
    run_command(command, build_dir)

    if install:
        command = command + ["install"]
    run_command(command, build_dir)  
    
    
def read_config_file(file_path):
    config = {}
    
    if not os.path.exists(file_path):
        sys.exit("ERROR: Invalid path: %s\n" % file_path)
    
    try:
        with open(file_path, 'r') as f:
            for line in f:
                if len(line.strip()) == 0:  # skip empty lines
                    continue
                if line.strip().startswith("#"): # skip comments
                    continue
                values = line.split("=")
                if len(values) < 2:
                    sys.exit("ERROR: Incorrect line in configuration file: %s\n" % line)
                config[values[0].strip()] = "=".join(values[1:]).strip()
                
    except:
        sys.exit("ERROR: Incorrect configuration file: %s\n" % file_path)
    
    return config
    
    
def clone_additional_code(args, config):
    if args.git != None:
        config["git_reprompi_bench"] = args.git
    if args.sha1 != None:
        config["sha1_reprompi_bench"] = args.sha1
    
    if not os.path.exists(config["build_dir"]):
        print "Creating build code directory: %s" % (config["build_dir"])
        os.makedirs(config["build_dir"])
        
    reprompi_repo_path = os.path.join(config["build_dir"], os.path.basename(config["git_reprompi_bench"]).split(".")[0])    
    reprompi_repo_path = os.path.join(base_path, reprompi_repo_path)

    print "%s\n## Copying/Cloning the ReproMPI code\n%s" % (COMMENT_STR, COMMENT_STR)
    if  os.path.exists(reprompi_repo_path):
        print "Removing existing ReproMPI code directory: %s" % (reprompi_repo_path)
        shutil.rmtree(reprompi_repo_path)
        
        
    try:
        routput = subprocess.check_output(["git", "clone", "--no-checkout", config["git_reprompi_bench"]],
                                        cwd = config["build_dir"],
                                        stderr=subprocess.STDOUT
                                        )
        routput = subprocess.check_output(["git", "checkout", config["sha1_reprompi_bench"]],
                                        cwd = reprompi_repo_path,
                                        stderr=subprocess.STDOUT
                                        )
        print routput
    except (OSError,subprocess.CalledProcessError) as e:
        if  os.path.exists(config["git_reprompi_bench"]):       # Try to copy code if it is located on a local path
            print "WARNING: Clone failed: %s" % e
            print "Trying to copy code from %s" % config["git_reprompi_bench"]
            try:
                shutil.copytree(config["git_reprompi_bench"], reprompi_repo_path)
            except OSError as e:
                print e
                sys.exit("ERROR: Failed to obtain ReproMPI code from: %s" % config["git_reprompi_bench"])
            
        else: # code is not on a local path - nothing to do
            print "Clone failed: %s" % e
            sys.exit("ERROR: Failed to obtain ReproMPI code from: %s" % config["git_reprompi_bench"])
    print "Done."
    
    print "\n\nReproMPI code was cloned here: %s" % reprompi_repo_path
    
    
def config_code(args, config):
    
    # use command-line arguments to override configuration   
    if not "reprompi_cmake_options" in config: 
        config["reprompi_cmake_options"] = ""
    if args.rdtscp != None and args.rdtscp == True:
        config["reprompi_cmake_options"] += " -DENABLE_RDTSCP=ON"
    if args.cpufreq != None:
        config["reprompi_cmake_options"] += " -DFREQUENCY_MHZ=%d" % (args.cpufreq)
        
    if args.synctype != None:
        dissemb = "OFF"
        hca = "OFF"
        jk = "OFF"
        sk = "OFF"
        if args.synctype == "dissemination_barrier":
            dissemb = "ON"
        elif args.synctype == "hca":
            hca = "ON"
        elif args.synctype == "jk":
            jk = "ON"
        elif args.synctype == "skampi":
            sk = "ON"
        config["reprompi_cmake_options"] += " -DENABLE_BENCHMARK_BARRIER=%s -DENABLE_WINDOWSYNC_HCA=%s -DENABLE_WINDOWSYNC_JK=%s -DENABLE_WINDOWSYNC_SK=%s" % (dissemb, hca, jk, sk)


    if not "mpidatatybe_cmake_options" in config: 
        config["mpidatatybe_cmake_options"] = ""
    if args.compilertype != None:
        platformfile = "default.cmake"
        if args.compilertype == "cray":
            platformfile = "CrayXC40.cmake"
        elif args.compilertype == "bgq":
            platformfile = "BlueGeneQ_IBMcompiler.cmake"
        elif args.compilertype == "intel":
            platformfile = "intel_compiler.cmake"
        
        config["reprompi_cmake_options"] += "-DINCLUDE_PLATFORM_CONFIG_FILE=platform_files/%s" % platformfile
        config["mpidatatybe_cmake_options"] += "-DINCLUDE_PLATFORM_CONFIG_FILE=platform_files/%s" % platformfile
        
    
    
    if os.path.exists(config["build_dir"]):
        if args.clean == True:
            print "Removing existing build code directory: %s" % (config["build_dir"])
            shutil.rmtree(config["build_dir"])
    
    if not os.path.exists(config["build_dir"]):
        print "Creating build code directory: %s" % (config["build_dir"])
        os.makedirs(config["build_dir"])
    
    reprompi_repo_path = os.path.join(config["build_dir"], os.path.basename(config["git_reprompi_bench"]).split(".")[0])    
    reprompi_repo_path = os.path.join(base_path, reprompi_repo_path)
    
    skipped_clone = False
    if args.clean == True or args.clone == True:
        clone_additional_code(args, config)
    else:
        skipped_clone = True
        print "Skipped ReproMPI repository cloning."
        if not os.path.exists(reprompi_repo_path):
            sys.exit("ERROR: The ReproMPI code cannot be found at: %s\nPlease clone the code with \"./build.py clone\"" % reprompi_repo_path)

    
    # generate the datatype benchmark code
    print "%s\n## Generating benchmarking code for mpi-datatybe\n%s" % (COMMENT_STR, COMMENT_STR)
    gen_script_path = os.path.join(reprompi_repo_path, config["code_generator_script"])
    if not os.path.exists(gen_script_path):
        sys.exit("ERROR: Git clone failed or wrong SHA1 downloaded - cannot find the code generation script in: \n\t%s\n" % gen_script_path)
    
    datatype_gen_code_path = os.path.join(config["build_dir"], config["gen_code_dirname"])
    datatype_gen_code_path = os.path.join(base_path, datatype_gen_code_path)
    command = [gen_script_path, 
               "-d", SOURCE_DIR,
               "-o", datatype_gen_code_path,
               "-l", config["path_to_list_of_source_files"]
               ]
    run_command(command, base_path)
    print "Done."
    
    print "%s\n## Configuring ReproMPI\n%s" % (COMMENT_STR, COMMENT_STR)
    cmake_options_str = ""
    if "reprompi_default_cmake_options" in config:
        cmake_options_str = cmake_options_str + " " + config["reprompi_default_cmake_options"]
    if "reprompi_cmake_options" in config:
        cmake_options_str = cmake_options_str + " " + config["reprompi_cmake_options"]
    
    cmake_source_code(reprompi_repo_path,  
                          cmake_options_str)
    print "Done."
    
    
    print "%s\n## Configuring MPI-datatybe\n%s" % (COMMENT_STR, COMMENT_STR)
    cmake_options_str = "-DREPRO_MPI_BENCHMARK_DIR=%s" % (reprompi_repo_path)
    if "mpidatatybe_default_cmake_options" in config:
        cmake_options_str = cmake_options_str + " " + config["mpidatatybe_default_cmake_options"]
    if "mpidatatybe_cmake_options" in config:
        cmake_options_str = cmake_options_str + " " + config["mpidatatybe_cmake_options"]
    cmake_source_code(datatype_gen_code_path,  
                          cmake_options_str)
    print "Done."
            
    print "\n\nThe configured benchmarks can be found here:"
    print "  - ReproMPI: %s" % reprompi_repo_path
    print "  - mpi-datatybe (including the generated benchmarking code): %s\n" % datatype_gen_code_path



def compile_code(args, config):
    reprompi_repo_path = os.path.join(config["build_dir"], os.path.basename(config["git_reprompi_bench"]).split(".")[0])    
    reprompi_repo_path = os.path.join(base_path, reprompi_repo_path)
    
    datatype_gen_code_path = os.path.join(config["build_dir"], config["gen_code_dirname"])
    datatype_gen_code_path = os.path.join(base_path, datatype_gen_code_path)

    if not os.path.exists(config["build_dir"]):
        sys.exit("ERROR: Build directory does not exist. Please run \"./build.py configure\" first.\n")
    if not os.path.exists(reprompi_repo_path):
        sys.exit("ERROR: The ReproMPI code does not exist in %s\nPlease run \"./build.py configure\" first.\n" % reprompi_repo_path)        
    if not os.path.exists(datatype_gen_code_path):
        sys.exit("ERROR: The mpi-datatybe generated code does not exist in %s\nPlease run \"./build.py configure\" first.\n" % datatype_gen_code_path)
            
    print "%s\n## Compiling ReproMPI\n%s" % (COMMENT_STR, COMMENT_STR)
    compile_source_code(reprompi_repo_path,
                        install = True)
    print "Done.\n"
    print "%s\n## Compiling mpi-datatybe\n%s" % (COMMENT_STR, COMMENT_STR)
    compile_source_code(datatype_gen_code_path)
    print "Done.\n"
    
    print "\n\nThe compiled benchmarks can be found here:"
    print "  - ReproMPI: %s" % reprompi_repo_path
    print "  - mpi-datatybe (including the generated benchmarking code): %s" % datatype_gen_code_path
    
    print "\n\nHow to run mpi-datatybe:"
    print "  export LD_LIBRARY_PATH=%s:$LD_LIBRARY_PATH" % os.path.join(reprompi_repo_path, "lib")
    print "  cd %s" % datatype_gen_code_path
    print """  mpirun -np 4 ./bin/reprompibench --params=b:MPI_INT --params=root:0 \\
               --params=test_type:datatype --params=pattern:bcast --params=layout:tiled \\
               --params=A:1 --params=B:16 --params=nbytes_list:256 --nrep=10 
        """   
      
  
def config_and_compile_code(args, config):  
    clone_additional_code(args, config)
    config_code(args, config)
    compile_code(args, config)




if __name__ == "__main__":    
    
    # Command-line options for configuration
    clone_args_parser = argparse.ArgumentParser(add_help=False)
    clone_args_parser.add_argument('--git', type=str, 
                              action='store',
                              help = 'URL or path to git repository')
    clone_args_parser.add_argument('--sha1', type=str, 
                              action='store',
                              help = 'commit SHA1')

    configure_args_parser = argparse.ArgumentParser(add_help=False)
    configure_args_parser.add_argument('--synctype', type=str, 
                                choices=['mpi_barrier', 'dissemination_barrier', 'hca', 'jk', 'skampi'],
                                action='store',
                                help = 'select synchronization method [default: MPI_Barrier]')
    configure_args_parser.add_argument('--compilertype', type=str, 
                                choices=['cray', 'bgq', 'intel', 'default'],
                                action='store',
                                help = 'select compiler [default: mpicc needs to be available in the path]')
    
    
    timing_group = configure_args_parser.add_argument_group(title = "Timing options - override the settings provided in the \"build.conf\" file",
                                                            description = "Change default ReproMPI time measurement settings (MPI_Wtime)")
    timing_group.add_argument('--rdtscp',
                              action='store_true',
                              help = 'enable RDTSCP-based timing') 
    timing_group.add_argument('--cpufreq', type=int, 
                              action='store',
                              help = 'set maximum CPU frequency in MHz [default: 2300 MHz]')     
    
    cleanup_group = configure_args_parser.add_argument_group()
    cleanup_group.add_argument('--clean', 
                              action='store_true',
                              dest = "clean",
                              help = 'remove existing build')
    cleanup_group.add_argument('--clone', 
                              action='store_true',
                              dest = "clone",
                              help = 'clone the ReproMPI repository')
       

    
    parser = argparse.ArgumentParser(description = "MPI-datatybe builder",
                                    epilog = "See \"config/build.conf\" for the full list of default configuration settings.")
    subparsers = parser.add_subparsers(title='Available sub-commands',
                                       dest='subparser_name'
                                       )

    clone_parser = subparsers.add_parser('clone', 
                                        help='clone ReproMPI benchmark code',
                                        parents=[clone_args_parser]
                                        )
    clone_parser.set_defaults(process_func=clone_additional_code)
    
    config_parser = subparsers.add_parser('configure', 
                                        help='generate code and configure build',
                                        parents=[clone_args_parser, configure_args_parser]
                                        )
    config_parser.set_defaults(process_func=config_code)
    
    compile_parser = subparsers.add_parser('compile', 
                                        help='compile the generated  mpi-datatybe code')
    compile_parser.set_defaults(process_func=compile_code)
        
    
    all_parser = subparsers.add_parser('all',
                                       help='generate, configure and compile mpi-datatybe', 
                                       parents=[clone_args_parser, configure_args_parser]
                                      )
    all_parser.set_defaults(process_func=config_and_compile_code)    
     
    # parse all arguments
    args = parser.parse_args()
    
    # read default configuration
    default_config = read_config_file(DEFAULT_CONFIG_FILE)
    # execute command
    args.process_func(args, default_config)
    


