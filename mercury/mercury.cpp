//#include <bits/stdc++.h>
#include <iostream>
#include <any>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cxxabi.h>
#include <memory>
#include <vector>
#include <sstream>
#include <cctype>
#include <variant>
#include <array>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <functional>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

bool in(std::string inp, std::vector<std::string> op){
    for (int i = 0; i < op.size() ; i ++){
    if (inp == op[i]){
        return true;
            //Do Stuff
    }}
    return false;
}

std::string replace(std::string s, std::string from, std::string to)
{
    if(!from.empty())
        for(std::string::size_type pos = 0; (pos = s.find(from, pos) + 1); pos += to.size())
            s.replace(--pos, from.size(), to);
    return s;
}


#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
#elif __linux__
    #include <unistd.h>
#endif

std::filesystem::path getExecutablePath() {
    char buffer[1024];
    
    #ifdef _WIN32
        GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    #elif __APPLE__
        uint32_t size = sizeof(buffer);
        _NSGetExecutablePath(buffer, &size);
    #elif __linux__
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) buffer[len] = '\0';
    #endif

    return std::filesystem::canonical(std::filesystem::path(buffer));
}


class ArgParser {
    public:
    ArgParser(char** argv,int argc): argv(argv), argc(argc){
    
    }
    bool has(std::string value){
        for (int i = 0; i != argc; i++){
            if (value == std::string(argv[i])){
                return true;
            }
        }
        return false;
    }
    void parse(){
        for (int i = 0; i != argc; i++){
            auto key = std::string(argv[i]);
            if(in(key,sflags)){

                std::string val;
                if (argv[i+1][0] == '\''){
                    while (argv[i+1][std::string(argv[i+1]).size()-1] != '\'' ){
                        val += std::string(argv[i+1]) + " ";
                        i++;

                    }
                    val += std::string(argv[i+1]);
                    val = replace(val,"'","\"");
                } else {
                    val = argv[i+1];
                }
                if (has("-sflags") || has("-flags")){
                    std::cout << key << " : " << val << std::endl;
                }
                if (values.find(key) == values.end()){
                    values[key] = std::vector<std::string>{std::string(val)};
                } else {
                    values[key].push_back(val);
                }

            i++;
            } else {
                if (has("-flags")){
                    std::cout << std::string(argv[i]) << std::endl;
                }
            }
            
        }

    }

    std::unordered_map<std::string,std::vector<std::string>> values;
    std::vector<std::string> sflags{"tundrac-path"};
    char** argv;
    int argc;
    private:
};
char** chardummy = new char*[3];
auto AP = ArgParser(chardummy,9);


void setTempEnvVar(std::string name, std::string value) {
#ifdef _WIN32
    _putenv((name + "=" + value).c_str());  // Windows
#else
    setenv(name.c_str(), value.c_str(), 1); // Linux/macOS
#endif
}

int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        std::cerr << "Error getting terminal size" << std::endl;
        return -1;
    }
    return csbi.srWindow.Right - csbi.srWindow.Left + 1; 
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        std::cerr << "Error getting terminal size" << std::endl;
        return -1;
    }
    return w.ws_col;  
#endif
}

#include <zlib.h>
bool downloadFile(const std::string url, const std::string outputPath) {
    std::string command = "curl -o " + outputPath + " " + url;
    return system(command.c_str()) == 0;
}

bool decompressGz(const std::string inputPath, const std::string outputPath) {

    gzFile inFile = gzopen(inputPath.c_str(), "rb");
    if (!inFile) {
        std::cerr << "Failed to open .gz file\n";
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create output file\n";
        gzclose(inFile);
        return false;
    }

    char buffer[4096];
    int bytesRead;
    while ((bytesRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
        outFile.write(buffer, bytesRead);
    }

    gzclose(inFile);
    outFile.close();
    return true;
}


const std::string red_block = "\033[31m";    // Red blk
const std::string green_block = "\033[32m";  // Green blk
const std::string yellow_block = "\x1b[33m";
const std::string reset = "\033[0m";
const std::string fcircle = "●";
const std::string circle = "◯";
std::unordered_map<std::string,std::string> opt2flags = {{"no_cc","-nocc"},{"show_cc","-showcc"},{"verbose","-v"},{"show_os_info","-showOSinfo"},{"suppress_warn","-nowarns"}};
std::unordered_map<std::string,std::string> em2ext = {{"object",".o"},{"llvm-ir",".ll"}};
std::unordered_map<std::string,std::string> targreg = {{"WindowsMsvc","x86_64-pc-windows-msvc"},{"WindowsMingw","x86_64-w64-mingw32"}};
std::vector<std::string> possibleexts = {".t4"};


std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

class TOMLParser {
public:
    void parseTOML(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filepath);
        }

        std::string line;
        std::string currentHeading;
        while (std::getline(file, line)) {
            line = trim(line);

            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;

            // Check for a heading [section]
            if (line[0] == '[' && line.back() == ']') {
                currentHeading = line.substr(1, line.size() - 2);
                data[currentHeading] = std::unordered_map<std::string, std::string>();
            } 
            // Check for key = value pairs
            else if (!currentHeading.empty() && line.find('=') != std::string::npos) {
                auto delimiterPos = line.find('=');
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));

                // Handle lists (assuming they are empty or strings for simplicity)
                if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
                    std::vector<std::string> listValues;
                    std::string content = value.substr(1, value.size() - 2);
                    std::stringstream ss(content);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        item = trim(item);
                        if (!item.empty() && item.front() == '"' && item.back() == '"') {
                            item = item.substr(1, item.size() - 2); 
                        }
                        listValues.push_back(item);
                        
                    }
                    lists[currentHeading][key] = listValues;
                } else if (value.front() == '"' && value.back() == '"') {

                    value = value.substr(1, value.size() - 2);
                    data[currentHeading][key] = value;
                }
            }
        }
        file.close();
    }

    std::unordered_map<std::string, std::string> getHeading(const std::string& heading) {
        if (data.find(heading) != data.end()) {
            return data[heading];
        } else {
            throw std::runtime_error("Heading not found: " + heading);
        }
    }

    std::vector<std::string> getList(const std::string& heading, const std::string& key) {
        if (lists.find(heading) != lists.end() && lists[heading].find(key) != lists[heading].end()) {
            return lists[heading][key];
        } else {
            throw std::runtime_error("List not found for heading: " + heading + ", key: " + key);
        }
    }


    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> lists;
private:
};

std::string parseTarg(TOMLParser tparser,std::string tar,std::string* llvm_target,bool* showTundrac){
        std::string structure_Tundrac;
        auto tardata = tparser.getHeading(tar);
        if (tparser.lists[tar].find("ccflags") != tparser.lists[tar].end()){
        for (auto& opt: tparser.getList(tar,"ccflags")){
            structure_Tundrac += " ccflags \"" + opt + "\"";
        }
        }
        if (tparser.lists[tar].find("options") != tparser.lists[tar].end()){
            for (auto& opt: tparser.getList(tar,"options")){
                
                if (opt2flags.find(opt) == opt2flags.end()){
                    if (opt == "show_tundrac"){
                        *showTundrac = true;
                    }else {
                    throw std::runtime_error("Unrecognised option: " + opt);
                    }
                } else {
                //std::cout << "added option: " << opt2flags[opt] << std::endl;

                structure_Tundrac += " " + opt2flags[opt];
                }
            }
            }
            if (tardata.find("toolchain") != tardata.end()){
                structure_Tundrac += " toolchain \"" + tardata["toolchain"] + "\"";
            
            }
            if (tardata.find("cc") != tardata.end()){
                structure_Tundrac += " cc \"" + tardata["cc"] + "\"";
            
            }
            if (tparser.lists[tar].find("libpaths") != tparser.lists[tar].end()){
            for (auto& opt: tparser.getList(tar,"libpaths")){
                structure_Tundrac += " linkpath \"" + opt + "\"";
            }
            }
            if (tparser.lists[tar].find("libs") != tparser.lists[tar].end()){
            for (auto& opt: tparser.getList(tar,"libs")){
                structure_Tundrac += " linklib \"" + opt + "\"";
            }
            }

            if (tardata.find("llvm_target") != tardata.end()){
                if (tardata.find("convention") != tardata.end()){
                    *llvm_target = tardata["convention"];
                    structure_Tundrac += " ccflags \" --target=" + tardata["llvm_target"] + "\"";
                } else {
                    *llvm_target = tardata["llvm_target"];
                }
            
            } else {
                if (targreg.find(tar) != targreg.end()){
                    *llvm_target = targreg[tar];

                }
            }
    return structure_Tundrac;
}   

TOMLParser build(int argc, char* argv[],bool launchnative=false){
        auto start_time = std::chrono::steady_clock::now();
        int fails=0;
        int builds=0;
        std::string tundrac_path = "tundrac";
        if (AP.has("tundrac-path")){
            tundrac_path = AP.values["tundrac-path"][0];
        }
        std::vector<std::string> failsstr;
        TOMLParser tparser;
        std::string build_dir = "build";
        tparser.parseTOML("Package.toml");
        std::string name = tparser.getHeading("project")["name"];
        std::string ver = tparser.getHeading("project")["version"];
        std::string typing = tparser.getHeading("project")["type"];
        std::cout << "Building project " << name << ".\n";
        auto outputs = tparser.getHeading("outputs");
        std::filesystem::create_directory(std::filesystem::absolute(build_dir));
        std::filesystem::create_directory(std::filesystem::absolute(build_dir + "/emissions"));
        std::filesystem::create_directory(std::filesystem::absolute(build_dir + "/" + name + ver));
        std::filesystem::create_directory(std::filesystem::absolute(build_dir + "/emissions/" + name + ver));
        auto targets = tparser.getList("targets","targets");
        
         
        for (auto& tar : targets){
        bool showTundrac = false;
        std::string ltarg = "";
        
        std::string llvm_target = ltarg;
        std::string structure_Tundrac=" ";
        auto emissions = tparser.getHeading("emissions");

        
        std::filesystem::create_directory(std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar));
        std::filesystem::create_directory(std::filesystem::absolute(build_dir + "/emissions/" + name + ver + "/" +  tar));
        std::unordered_map<std::string, std::string> tardata;
        if (tparser.data.find(tar) != tparser.data.end()){
            structure_Tundrac += " " + parseTarg(tparser,tar,&llvm_target,&showTundrac);
            tardata = tparser.getHeading(tar);
            //std::cout << "Got tardata" << std::endl;


        }
        if (tparser.data.find("global") != tparser.data.end()){
            structure_Tundrac += " " + parseTarg(tparser,"global",&llvm_target,&showTundrac);



        }
        structure_Tundrac += " t4path \"" + std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "\"";
        if (std::getenv("Tundra_STD_PATH")){
            structure_Tundrac += " t4path \"" + std::filesystem::absolute(std::getenv("Tundra_STD_PATH")).string() + "\"";
        }
        auto prev = structure_Tundrac;
        

        for (const auto& [key,value] : outputs){
            builds +=1 ;
            structure_Tundrac = prev;
            std::string tarout = "";
            std::string outextension =""; // out extensions by Tundrac not us
            if (tparser.data.find(tar) != tparser.data.end()){
                if (tardata.find(value + "_suffix") != tardata.end()){
                    tarout = "." + tardata[value + "_suffix"];
                }
                
            }
            if (tparser.lists["emissions"].find(key) != tparser.lists["emissions"].end()){
                
                auto emlist = tparser.getList("emissions",key);
                for (auto& em : emlist){
                structure_Tundrac += " emit " + em;
                //tarout = ".t4";
                }
            }
            if (outputs[key] == "t4"){
                structure_Tundrac += " -bundle";
                outextension = ".t4";
            }
            for (auto& h : tparser.getList("externals","header_paths")){
                structure_Tundrac += " headerpath \"" + h + "\"";
            }
            
            
            std::cout << green_block + circle + reset << " Building src/" << key << " -> " << value << "\n";
            std::string ccmd = tundrac_path + " compile src/" + key + ".ta "  + " target " + (llvm_target == "" ? tar : llvm_target)   + structure_Tundrac + " output \"" + std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key + tarout + "\"" + " typing " + typing;
            //std::cout << ccmd << std::endl;
            if (showTundrac){
                std::cout << ccmd << std::endl;
            }
            possibleexts.push_back(tarout);
            for (auto& oext : possibleexts){
            if (std::filesystem::exists(std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key +  oext)){
                std::filesystem::remove(std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key +  oext);
            }
            }
            system(ccmd.c_str());
            auto all = possibleexts.size();
            for (auto& oext: possibleexts){
            if (!std::filesystem::exists(std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key + oext)){
                all -= 1;

            }
            }
            if (all == 0){
                fails += 1;
                failsstr.push_back(key);
                continue;
            }
            

            
            if (tparser.lists["emissions"].find(key) != tparser.lists["emissions"].end()){
                auto emlist = tparser.getList("emissions",key);
                for (auto& em : emlist){
                auto empath = std::filesystem::absolute("src/" + key + em2ext[em]);
                std::filesystem::rename(empath,std::filesystem::absolute(build_dir + "/emissions/" + name + ver + "/" + tar + "/" + key + em2ext[em]));

                }
            }
            if (tar == "native" && launchnative){
                if (outputs.find(key) != outputs.end() && outputs[key] != "exec"){
                } else {
                    std::cout << yellow_block + circle + reset << " Running " <<std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key + tarout << std::endl;
                    auto ccircle = "\033[34m" + circle + reset;
                    std::ostringstream oss;
                    for (int i = 0; i < getTerminalWidth()/2; i++) {
                        oss << ccircle + " ";
                    }
                    std::cout << oss.str() <<std::endl;
                    system(std::string(std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key + tarout).c_str());
                    std::cout << oss.str() <<std::endl;
                    std::cout<< green_block + fcircle + reset  << " Ran " <<std::filesystem::absolute(build_dir + "/" + name + ver + "/" + tar).string() + "/" + key + tarout << std::endl;
                    
                }
            }
            
            
            

        }
        if (fails == 0){
            std::cout << green_block << fcircle << reset << " Finished build for target: " << tar << "\n";
        } else {
            if (builds != fails){
                std::cout << yellow_block << fcircle << reset <<  " " << builds-fails <<" Builds succeeded for target: " << tar << "\n";
            }
            std::cout << red_block << fcircle << reset << " " <<  fails <<" Builds failed for target: " << tar << "\n";
            std::string fail_str = "";
            int ct =0;
            for (auto& f: failsstr){
                fail_str += " " + f + (f != failsstr[failsstr.size()-1] ? "," : "");
                if (ct == 3){
                    fail_str += " ...";
                    break;
                }
                ct += 1;
            }
            std::cout << red_block << fcircle << reset << " Failed builds for '" << tar << "' include:\n";
            std::cout << red_block << fcircle << reset << fail_str << "\n";
            
        }
        }
        //std::filesystem::rename(std::filesystem::absolute(build_dir + "/"),std::filesystem::absolute("build/"));
        std::string timer;
        auto end_time = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
        int seconds = (static_cast<int>(elapsed_seconds.count()) % 60) / 60;
        if (minutes == 0 && (static_cast<int>(elapsed_seconds.count()) % 60) == 0){
            timer = std::to_string(static_cast<int>(elapsed_seconds.count()* float(100))) + "ms";
        }
        else if (minutes == 0){
            timer = std::to_string(static_cast<int>(elapsed_seconds.count())) + "s";
        } 
        else {
            timer = std::to_string(minutes) + "." + std::to_string(seconds) + "m";
        }
        std::cout << green_block << fcircle << reset << " Finished build " << name << " in " << timer << "\n";
        return tparser;
}



int main(int argc, char* argv[]){
    AP.argv = argv;
    AP.argc = argc;
    AP.parse();
    bool prod = false;
    if (argc < 2){
        std::cout << "Mercury (v Arcadia)\nhttps://github.com/Tundra-t4/Tundra\n(Tip: run 'Mercury help' for help)\n";
        return 1;

    }
    auto exe_path = std::filesystem::absolute(getExecutablePath());
    setTempEnvVar("Tundra_ROOT",exe_path.parent_path());
    setTempEnvVar("Tundra_STD_PATH",exe_path.parent_path().string() + "/std/");
    if (std::string(argv[1]) == "new"){
        std::string name = argv[2];
        std::filesystem::create_directory(std::filesystem::absolute(name));
        std::string src = name + "/src/";
        std::filesystem::create_directory(std::filesystem::absolute(src));
        std::string cmd = "cd " + name + " && git init";
        system(cmd.c_str());
        std::cout << "Created project " << name << " (with git vc)" << std::endl;
        std::string path = name + "/Package.toml";
        std::string lpath = name + "/Package.lock";
        std::string ppath = name + "/src/" + name + ".ta"; 
        std::ofstream file(path);
        std::ofstream fp(ppath);
        std::ofstream lfp(lpath);

        file << "[project]\nname = \"" + name + "\"\nversion = \"0.1.0\"\ntype = \"static\"\nauthors = []\n\n[outputs]\n" + name + " = " + "\"target_file_format\"\n#eg. hello_world =  exec"  + "\n\n[targets]\ntargets = [\"native\"]\n\n[emissions]\n#eg. hello_world =  [\"object\"]\n\n[externals]\nheader_paths = []\n\n[artifacts]";
        file.close();
        fp.close();
        lfp.close();
        return 1;
        
    }
    if (std::string(argv[1]) == "build" && argc > 2){
        if (std::string(argv[2]) == "run"){
        build(argc,argv,true);
        return 0;
        }

    }
    if (std::string(argv[1]) == "build"){
        build(argc,argv);
        return 0;

    }
    if (std::string(argv[1]) == "version"){
        std::cout << "Mercury: Anthology no. 2025b (Arcadia)" << std::endl;
    }

    if (std::string(argv[1]) == "up" || std::string(argv[1]) == "update"){
        std::cout << "Mercury: Anthology no. 2025b (Arcadia)" << std::endl; // later
    }

    if (std::string(argv[1]) == "help"){
        std::cout << (
        "Mercury is a project/package manager for Tundra\n"   
        "Mercury Commands List:\n\n"
        "new: Create a new project\neg.    \"Mercury new HelloWorld\"\n"
        "build: Build a project\neg.    \"Mercury build\"\n"
        "build run: Build a project and run the native executables\neg.    \"Mercury build run\"\n"
        "clean: Clean the build folder\neg.    \"Mercury clean\"\n"
        "version: Get Mercury's version\neg.    \"Mercury version\"\n"
        "update (up): Update Mercury to the latest version\neg.    \"Mercury update\" or \"Mercury up\"\n"
        )<< std::endl;
    }

    if (std::string(argv[1]) == "run"){
        TOMLParser tparser = build(argc,argv);
        for (const auto& [key,value] : tparser.getHeading("targets")){
            std::cout << "running src/" << value << ":\n";
            std::string runcmd = "cd src && .\\" + value;
            system(runcmd.c_str());
            return 0;
        } 
    }

    if (std::string(argv[1]) == "clean"){
        if (argc > 2){
            std::cout << green_block << circle << reset << " Removing target " + std::string(argv[2]) << std::endl;
            std::filesystem::remove_all(std::filesystem::absolute("build/" + std::string(argv[2])));
            std::cout << green_block << fcircle << reset << " Removed target " + std::string(argv[2]) << std::endl;
        } else {
        std::cout << green_block << circle << reset << " Removing build." << std::endl;
        std::filesystem::remove_all(std::filesystem::absolute("build/"));
        std::cout << green_block << fcircle << reset << " Removed build." << std::endl;
        }

    }




    
    return 0;
}