#include <iostream>
#include <string>
#include <stdlib.h>
#include "json.hpp"
#include <fstream>
#include <locale>
#define NOMINMAX
#include <limits>
#include <filesystem>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define DISPLAY_BACKEND "sdl"
#else
#define DISPLAY_BACKEND "gtk"
#endif

#ifdef _WIN32
#define CLEAR "CLS"
#else
#define CLEAR ""
#endif

using json = nlohmann::json;

#ifndef _WIN32
int run_qemu(const std::vector<std::string>& args){
    pid_t pid = fork();

    if (pid == 0){
        std::vector<char*> c_args;
        for(const auto& s : args)
            c_args.push_back(const_cast<char*>(s.c_str()));
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        perror("execvp");
        _exit(1);
    } else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
        return status;
    } else {
        perror("fork");
        return -1;
    }
};
#endif

#ifdef _WIN32
int runcommandWindows(const std::string& cmd){
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::vector<char> buffer(cmd.begin(),cmd.end());
    buffer.push_back('\0');

    if (!CreateProcessA(nullptr,buffer.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&si,&pi)){
        return -1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
#endif

int runCommand(const std::vector<std::string>& args){
    #ifdef _WIN32
        std::string cmd;
        for (const auto& s : args)
            cmd += "\"" + s + "\" ";
            return runcommandWindows(cmd);
    #else
        return run_qemu(args);
    #endif
}

struct ConfigVM{
    std::string RAM,NUCLEOS,DISCO,ISO,ACC;
};

void clearScreen(){
    #ifdef _WIN32
        system(CLEAR);
    #else
        std::cout << "\033[2J\033[H";
    #endif
};

bool CommandExists(const std::string& cmd){
    #ifdef _WIN32
        std::string test = "where " + cmd + " >nul 2>&1";
    #else
        std::string test = "which " + cmd + " >/dev/null 2>&1";
    #endif
        return std::system(test.c_str()) == 0;
};

std::filesystem::path getExeDir(char* argv0){
    return std::filesystem::absolute(argv0).parent_path();
};

int main(int argc, char* argv[]){
    auto exeDir = getExeDir(argv[0]);
    std::filesystem::path configPath = exeDir / "config.json";
    if (!CommandExists("qemu-system-x86_64")) {
        std::cerr << "[ADVERTENCIA] QEMU no está instalado o no está en el PATH\n";
            std::cin.get();;

    }
    setlocale(LC_ALL, "");
    ConfigVM config;
    char opcion;
    std::ifstream configjs(configPath.string());
    if (!configjs.is_open()){
        std::cerr << "ERROR: No se ha podido abrir el archivo 'config.json'" << std::endl;
        return 1;
    }
    
    json configuracion;

    try{
        configjs >> configuracion;
    } catch (json::parse_error& e){
        std::cerr << "Error al parsear JSON: " << e.what() << std::endl;
        return 1;
    }

    configjs.close();
    
    config.RAM = configuracion["RAM"];
    config.NUCLEOS = configuracion["NUCLEOS"];
    config.ISO = configuracion["ISO"];
    config.DISCO = configuracion["DISCO"];
    config.ACC = configuracion["ACC"];

    while (true){
        clearScreen();
        std::cout << "========== MENU QEMUQS ==========" << std::endl;
        std::cout << "1) Iniciar VM QEMU" << std::endl;
        std::cout << "2) Ver configuración actual" << std::endl;
        std::cout << "3) Configurar VM" << std::endl;
        std::cout << "4) Crear VM QEMU" << std::endl;
        std::cout << "x) Salir" << std::endl;
        std::cout << "\nOpción: ";

        std::cin >> opcion;
        clearScreen();
        
        if (std::cin.fail() || (opcion != '1' && opcion != '2' && opcion != '3' && opcion != '4' && opcion != 'x')) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Elegiste una opción inválida" << std::endl;
            std::cout << "Presiona ENTER para continuar...";
            std::cin.get();
            continue;
        }

        if (opcion == '1'){
            std::vector<std::string> args = {"qemu-system-x86_64", "-m", config.RAM + "G", "-smp", config.NUCLEOS, "-drive","file=" + config.DISCO + ",format=qcow2", "-name", "Máquina Virtual - QEMUQS by RyTro","-machine","q35","-display", DISPLAY_BACKEND, "-cpu", "max", "-vga", "virtio", "-accel", config.ACC};
            if (!config.ISO.empty()){
                args.push_back("-cdrom");
                args.push_back(config.ISO);
                args.push_back("-boot");
                args.push_back("once=d");
            } else {
                args.push_back("-boot");
                args.push_back("order=c");
            }
            runCommand(args);
        } 

        else if (opcion == '2'){
            std::cout << "========== CONFIGURACIÓN ACTUAL ==========" << std::endl;
            std::cout << "RAM: " << config.RAM << std::endl;
            std::cout << "NÚCLEOS: " << config.NUCLEOS << std::endl;
            std::cout << "RUTA ISO: " << config.ISO << std::endl;
            std::cout << "DISCO: " << config.DISCO << std::endl;
            std::cout << "ACELERACIÓN: " << config.ACC << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cin.get();;
            clearScreen();
        } 

        else if (opcion == '3'){
            std::cout << "========== CONFIGURACIÓN QEMUQS ==========" << std::endl;
            std::cout << "RAM: ";
            std::cin >> config.RAM;
            std::cout << "NÚCLEOS: ";
            std::cin >> config.NUCLEOS;
            std::cout << "RUTA ISO: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, config.ISO);
            std::cout << "DISCO: ";
            std::getline(std::cin, config.DISCO);
            std::cout << "ACELERACIÓN (windows -> whpx || tcg; Linux -> kvm ): ";
            std::cin >> config.ACC;
            clearScreen();
            configuracion["RAM"] = config.RAM;
            configuracion["NUCLEOS"] = config.NUCLEOS;
            configuracion["ISO"] = config.ISO;
            configuracion["DISCO"] = config.DISCO;
            configuracion["ACC"] = config.ACC;

            std::ofstream out(configPath.string());
            out << configuracion.dump(4);
            out.close();
        }

        else if (opcion == '4'){
            std::string size,name,ruta;
            std::cout << "Cuántos GB de almacenamiento le quiere asignar a esta máquina virtual?: ";
            std::cin >> size;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Dónde quiere almacenar la VM?: ";
            std::getline(std::cin, ruta);
            std::cout << "Cómo quiere llamar a esta máquina virtual?: ";
            std::getline(std::cin, name);
            std::filesystem::path disco = std::filesystem::path(ruta) / (name + ".qcow2");
            
            int res = runCommand({"qemu-img","create","-f","qcow2",disco.string(),size+"G"});
            if (res != 0){
                std::cerr << "Error al crear disco virtual\n";
                std::cin.get();
            }
            config.DISCO = disco.string();
            configuracion["DISCO"] = config.DISCO;
            std::ofstream out(configPath.string());
            out << configuracion.dump(4);
            out.close();
        }

        else if (opcion == 'x'){
            std::cout << "Saliendo...";
            break;
        } 
    }
};