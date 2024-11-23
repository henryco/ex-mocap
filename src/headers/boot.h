//
// Created by henryco on 23/11/24.
//

#ifndef BOOT_H
#define BOOT_H

/**
 * Polymorphism not needed
 */
namespace ex::boot {

    int gui(int argc, char **argv, const char *project_path);

    int ttui(int argc, char **argv, const char *project_path);

}

#endif //BOOT_H
