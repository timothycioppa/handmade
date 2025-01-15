#ifndef _SCENE_PARSER_HPP
#define _SCENE_PARSER_HPP

#include "math_utils.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include "light.hpp"
#include "bsp.hpp"

void load_scene(const char* filename, bsp_tree & tree) ;

#endif