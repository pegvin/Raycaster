#ifndef RAYCASTER_WORLDMAP_H_INCLUDED_
#define RAYCASTER_WORLDMAP_H_INCLUDED_ 1
#pragma once

#define mapWidth 32
#define mapHeight 24
#define mapUnit 10
char map[mapHeight][mapWidth] = {
	"################################",
	"#                              #",
	"#   #                          #",
	"#   #            #####         #",
	"#   #                          #",
	"#         ####                 #",
	"#         #  #   ######        #",
	"#         #  #                 #",
	"#         #  #                 #",
	"#         ####                 #",
	"#                              #",
	"#         #########   ##########",
	"#         #########   #        #",
	"#         #           #        #",
	"#         #  #########         #",
	"#         #  #          #      #",
	"#            #         #       #",
	"#       ######        #        #",
	"#                    #         #",
	"#                     #        #",
	"#        #   #         #       #",
	"#       #     #         #      #",
	"#                              #",
	"################################",
};

#endif // RAYCASTER_WORLDMAP_H_INCLUDED_

