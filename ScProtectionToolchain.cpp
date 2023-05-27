#include "ProEditLib/src/ProEdit.h"
#include "TinyMapLib/src/TinyMap.h"
#include "SpecialProtectorLib/src/SpecialProtector.h"
#include "SMLPLib/src/SMLP.h"
#include <Desktop.h>

int main(int argc, char** argv) {
    VisibleDesktopFactory vf;
    HiddenDesktopFactory hf;
    bool hiddenDesktop = true;
    bool printHelp = false;

    char* alg = nullptr;
    char* algA[100];
    memset(algA, 0, sizeof(algA));

    char* input = nullptr;
    char* output = nullptr;

    if (argc <= 1) {
        printHelp = true; 
    } else {
        for (int i = 1; i < argc; i++) {
            char* arg = argv[i];
            char* param = i + 1 < argc ? argv[i + 1] : nullptr;
            if (!strcmp(arg, "-i")) {
                input = param;
                i++;
            } else if (!strcmp(arg, "-o")) {
                output = param;
                i++;
            } else if (!strcmp(arg, "-a")) {
                alg = param;
                i++;
            } else if (!strcmp(arg, "-visible")) {
                hiddenDesktop = false;
            } else if (!strcmp(arg, "-hidden")) {
                hiddenDesktop = true;
            } else if (arg[0] == '-' && arg[1] == 'a') {
                int32 aa = atoi(&arg[2]);
                i++;
                if (aa >= 1 && aa < sizeof(algA)) {
                    algA[aa - 1] = param;
                } else {
                    LOG_ERROR("%s: invalid value", arg);
                    return 1;
                }
            }
        }

    }

    if (printHelp) {
        printf("ScProtectionToolchain\r\nMade by: iThief\r\n\r\n");
        printf("Usage: ScProtectionToolchain.exe -a <program> [-visible|-hidden(=default)] -i <input_scx> -o <output_scx> -a1...a100 program options");
        printf("\r\n\r\nAvailable programs:\r\n");
        
        // Proedit
        printf("proedit\r\n\tOptions:\r\n");
        printf("\t\t-a1 <username>\r\n");
        printf("\t\t-a2 <email>\r\n");

        // Tinymap
        printf("tinymap\r\n\r\n");

        // Special protector
        printf("special\r\n\r\n");

        printf("\r\n");
    } else {
        DesktopFactory* f = hiddenDesktop ? (DesktopFactory*) &hf : (DesktopFactory*) &vf;

        bool run = true;
        if (!input) {
            LOG_ERROR("Missing input parameter");
            run = false;
        }
        if (!output) {
            LOG_ERROR("Missing output parameter");
            run = false;
        }
        if (!alg) {
            LOG_ERROR("Missing alg parameter");
            run = false;
        }
        if (run) {
            if (!strcmp(alg, "proedit")) {
                if (!algA[0] || !algA[1]) {
                    LOG_ERROR("Missing a1 or a2 parameter");
                    run = false;
                }
                if (run) {
                    ProEdit p(f, algA[0], algA[1]);
                    if (p.Check()) {
                        run &= p.Protect(input, output);
                    } else {
                        run = false;
                    }
                }
            } else if (!strcmp(alg, "tinymap")) {
                if (run) {
                    TinyMap p(f);
                    if (p.Check()) {
                        run &= p.Protect(input, output);
                    } else {
                        run = false;
                    }
                }
            } else if (!strcmp(alg, "special")) {
                if (run) {
                    SpecialProtector p(f);
                    if (p.Check()) {
                        run &= p.Protect(input, output);
                    } else {
                        run = false;
                    }
                }
            } else if (!strcmp(alg, "smlp")) {
                if (run) {
                    SMLP p(f);
                    if (p.Check()) {
                        run &= p.Protect(input, output);
                    } else {
                        run = false;
                    }
                }
            }
        }
        if (!run) {
            LOG_ERROR("Failed to run protection tool");
        }
        return run ? 0 : 1;
    }
    return 0;
}
