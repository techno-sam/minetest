#!/usr/bin/python3

import pygame

dtt = [
    #X+ Y+ Z+ Z- Y- X-
    [2, 0, 4, 5, 1, 3],
    [4, 0, 3, 2, 1, 5],
    [3, 0, 5, 4, 1, 2],
    [5, 0, 2, 3, 1, 4],
    [2, 5, 0, 1, 4, 3],
    [4, 2, 0, 1, 3, 5],
    [3, 4, 0, 1, 5, 2],
    [5, 3, 0, 1, 2, 4],
    [2, 4, 1, 0, 5, 3],
    [4, 3, 1, 0, 2, 5],
    [3, 5, 1, 0, 4, 2],
    [5, 2, 1, 0, 3, 4],
    [0, 3, 4, 5, 2, 1],
    [0, 5, 3, 2, 4, 1],
    [0, 2, 5, 4, 3, 1],
    [0, 4, 2, 3, 5, 1],
    [1, 2, 4, 5, 3, 0],
    [1, 4, 3, 2, 5, 0],
    [1, 3, 5, 4, 2, 0],
    [1, 5, 2, 3, 4, 0],
    [3, 1, 4, 5, 0, 2],
    [5, 1, 3, 2, 0, 4],
    [2, 1, 5, 4, 0, 3],
    [4, 1, 2, 3, 0, 5]
]

texdir = "/home/sam/Minetest/minetest/cache/media_unscrambled/"

data = open("/home/sam/Minetest/minetest/clientmods/schematical/data/nodedef.txt").read().split("\n")

data = data[:-1]

textures = {}

tex_to_node = {}

blacklist_nodes = [
    "mfu:mfu",
    "mfu:mfu_active",
    "bones:bones",
    "ehlphabet:machine",
    "mesecons_detector:node_detector_off",
    "atm:wtt",
    "freezer:freezer",
    "currency:shop",
    "cottages:furnace_active",
    "farebox:farebox",
    "atm:atm",
    "mesecons_detector:node_detector_on",
    "cottages:furnace",
    "currency:safe",
    "towercrane:base",
    "mailbox:letterbox"
]

num = 0

for d in data:
    dat = d.split("/")
    if not dat[1] == 'facedir':
        continue
    if not dat[2] == 'normal':#in ['normal','nodebox','mesh']:
        continue
    #print(dat[0])
    num+=1
    node_name = dat[0]
    if node_name in blacklist_nodes or "marmor" in node_name:
        continue
    tiles = [dat[i] for i in range(3,9)]
    for i in range(len(tiles)):
        tex = tiles[i]
        t = tex.split("^")[0].replace("(","").replace(")","").replace("[","").replace("]","")
        if "_animated" in t:
            continue
        if "technic_lv" in t or "technic_mv" in t or "technic_hv" in t or "mesecons" in t or "technic" in t or "pipeworks" in t or "pacmine" in t or "marmor" in t:
            continue
        if t=='':
            continue
        try:
            textures[t]
        except KeyError:
            #print(texdir+t)
            try:
                im = pygame.image.load(texdir+t)
            except:
                continue
            textures[t] = [im,pygame.transform.average_color(im)]
        if dat[1]=='facedir':
            i = -1
        try:
            #if not dat[0] in tex_to_node[t]:
            tex_to_node[t].append((dat[0],i))
        except KeyError:
            tex_to_node[t] = [(dat[0],i)]

def diff(c1,c2):
    return abs(c1[0]-c2[0])+abs(c1[1]-c2[1])+abs(c1[2]-c2[2])

side = input("Which side? (X/Y/Z)(+/-)/ALL: ")
target = None
try:
    m = {}
    m["X+"] = 2
    m["Y+"] = 0
    m["Z+"] = 4
    m["Z-"] = 5
    m["Y-"] = 1
    m["X-"] = 3
    m["ALL"] = -1
    m[""] = -1
    side = m[side.upper()]
    target = pygame.image.load(input("Target image: "))
except:
    target = pygame.image.load(side)
    side = -1

def matches_side(texname,side):
    for dat in tex_to_node[texname]:
        if dat[1]==side or dat[1]==-1 or side==-1:
            return True
    return False

def get_best_tex(color):
    best_tex = ""
    best_dif = float("inf")
    for tex in textures:
        col = textures[tex][1]
        if diff(color,col)<best_dif and matches_side(tex,side):
            best_dif = diff(color,col)
            best_tex = tex
    return best_tex

res = 32
output = pygame.Surface([target.get_width()*res,target.get_height()*res])
node_counts = {}
#output.fill((255,255,0))
for x in range(target.get_width()):
    for y in range(target.get_height()):
        if target.get_at((x,y))[3]<=10:
            #pygame.draw.rect(output,(255,255,0,0),(x*res,y*res,res,res))
            continue
        t = get_best_tex(target.get_at((x,y)))
        node_name = tex_to_node[t][0][0]
        node_counts[node_name] = node_counts.get(node_name, 0) + 1
        #print(t)
        output.blit(pygame.transform.scale(textures[t][0],(res,res)),(x*res,y*res))
#output.set_colorkey((255,255,0))
pygame.image.save(output,"/home/sam/Minetest/minetest/util/pixart.png")

#screen = pygame.display.set_mode((output.get_width(),output.get_height()))
#screen.blit(output,(0,0))
#pygame.display.update()
#input("ENTER TO QUIT")
pygame.quit()


while len(node_counts) > 0:
    max_num = 0
    max_name = ""
    for node_name in node_counts:
        num = node_counts[node_name]
        if num>max_num:
            max_num = num
            max_name = node_name
    print(f"{max_name}: {max_num}")
    del node_counts[max_name]
