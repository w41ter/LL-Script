#include <iostream>
#include <cstring>

#include "driver.h"

namespace script
{
    bool Driver::parseArguments(int argc, char *argv[])
    {
        if (argc == 1)
        {
            usage();
            return false;
        }

        int count = 1;
        while (count < argc)
        {
            if (argv[count][0] == '-')
            {
                count = command(count, argv);
                if (count == -1)
                    return false;
                continue;
            }

            if (filename)
            {
                std::cout << "warning: file \"" << argv[count] << "\"be ignore" << std::endl;
                count++;
                continue;
            }
            filename = argv[count++];
        }

        if (!filename)
        {
            std::cout << "error: no input file" << std::endl;
            return false;
        }
        return true;
    }

    void Driver::usage()
    {
        std::cout << "Usage : [-op] filename" << std::endl;
        std::cout << "\t -dumpIR" << std::endl;
        std::cout << "\t -o" << std::endl;
    }

    int Driver::command(int count, char *argv[])
    {
        if (strcmp("-dumpIR", argv[count]) == 0)
        {
            dumpIR_ = true;
        }
		else if (strcmp("-dumpOpcode", argv[count]) == 0)
		{
			dumpOpcode_ = true;
		}
        else if (strcmp("-o", argv[count]) == 0)
        {
            optimized_ = true;
        }
        else
        {
            usage();
            return -1;
        }
        return count + 1;
    }
}