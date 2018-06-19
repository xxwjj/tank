#!coding:utf-8

import datetime
import csv
import os
import time
import re
import zipfile
import rarfile
import sys
from shutil import copyfile

OUTPUT_DIR = ".\\output"

# Exception definition
class NotSupportFileType(Exception):
    """Do not support this type of compress file."""

class NotFindGameClientFile(Exception):
    """System cant find the game client file."""


def un_rar(file_name):
    """unrar zip file"""
    file_name = os.path.abspath(file_name)
    old_path = os.getcwd()
    try:
        rar = rarfile.RarFile(file_name)
        if os.path.isdir(file_name + "_files"):
            pass
        else:
            os.mkdir(file_name + "_files")
        os.chdir(file_name + "_files")
        rar.extractall()
        rar.close()
    finally:
        os.chdir(old_path)


def un_zip(file_name):
    """unzip zip file"""
    file_name = os.path.abspath(file_name)
    zip_file = zipfile.ZipFile(file_name)
    if os.path.isdir(file_name + "_files"):
        pass
    else:
        os.mkdir(file_name + "_files")
    for names in zip_file.namelist():
        zip_file.extract(names,file_name + "_files/")
    zip_file.close()


def get_production_file_list(top_dir):
    top_dir = os.path.abspath(top_dir)
    file_list = []
    for file in os.listdir(top_dir):
        path = os.path.join(top_dir, file)
        if os.path.isfile(path) and (file.endswith(".zip") or file.endswith(".rar")):
            file_list.append(path)
    return file_list


def find_top_dir(top_dir):
    files = os.listdir(top_dir)
    if len(files) == 1 :
        return find_top_dir(os.path.join(top_dir, files[0]))
    elif len(files) > 1 and "gameclient.bat" in files:
         return top_dir
    raise NotFindGameClientFile



def uncompress_file(file):
    name = os.path.basename(file)
    if not os.path.exists(file+"_files"):
        if name.endswith(".rar"):
            un_rar(file)
        elif name.endswith(".zip"):
            un_zip(file)
        else:
            print("Do not support this type of compress file", name)
            raise NotSupportFileType
    return find_top_dir(file+"_files")


BAT_SCRIPT = '''
@echo off

pushd %%CD%%
cd /d %(server_path)s
start gameserver.bat %(map_name)s 127.0.0.1 6000
popd

pushd %%CD%%
cd /d %(player_path)s
start gameclient.bat "%(team_id)s" 127.0.0.1 6000 
popd

pushd %%CD%%
cd /d %(ai_path)s
start gameAI.bat 9999 127.0.0.1 6000 
popd
'''

def save_file(src, dest, map):
    src = os.path.abspath(src)
    dest = os.path.abspath(dest)
    copyfile(os.path.join(src, "replay.txt"), os.path.join(dest, map+"replay.txt"))
    copyfile(os.path.join(src, "data.csv"), os.path.join(dest, map+"data.csv"))


def add_exit(fname):
    with open(fname, 'r') as f:
        lines = f.readlines()
        last_line = lines[-1]
        if last_line.strip().lower() == "exit":
            return
    with open(fname, 'a') as f:
        f.writelines("\nexit")


def run_game(team_id, player_path, map_name):
    server_path = os.path.abspath("Tank/server")
    ai_path = os.path.abspath("Tank/client_AI")
    bat_script = BAT_SCRIPT % {"server_path": server_path, "ai_path": ai_path, "player_path": player_path,
                               "map_name": map_name, "team_id": team_id}

    with open("start_game.bat", "w") as f:
        f.write(bat_script)
    server_data_csv_path = os.path.join(server_path, "data.csv")
    try:
        os.remove(server_data_csv_path)
    except FileNotFoundError:
        pass

    add_exit(os.path.join(player_path, "gameclient.bat"))

    os.system("start_game.bat")

    for _ in range(100):
        time.sleep(1)
        if os.path.exists(server_data_csv_path):
            break
    else:
        return 0, 0, 0, "timeout"

    ai_score = 0
    player_score = 0
    status = "normal"
    with open(server_data_csv_path, newline='') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        for row in spamreader:
            if row[0] == team_id:
                player_score = row[3]
                status = row[2]
            elif row[0] == "9999":
                ai_score = row[3]
    save_file(server_path, os.path.join(OUTPUT_DIR, team_id), map_name[:-3])
    return player_score, ai_score, int(player_score) - int(ai_score), status


def run_games(team_id, player_path):
    map_list = ["map_chain.txt", "map_demo.txt", "map_empty.txt", "map_gold_rush.txt", "map_space_craft.txt"]
    score_list = []
    for map in map_list:
        score = run_game(team_id, player_path, map)
        score_list.append(score)
        print(">>> Score: %s vs %s \tMap:%s"% (score[0], score[1], map))
    return score_list


def get_team_id(file):
    name = os.path.basename(file)
    return name[:-4]


def write_result(filename, team_id, score_list):
    with open(filename, 'a', newline='') as csvfile:
        spamwriter = csv.writer(csvfile, delimiter=',',
                                quotechar='|', quoting=csv.QUOTE_MINIMAL)
        player_sum = 0
        ai_sum = 0
        item = []
        for s in score_list:
            player_sum += int(s[0])
            ai_sum += int(s[1])
            item.extend(s)
        total_diff = player_sum - ai_sum
        spamwriter.writerow([team_id]+item+[player_sum, ai_sum, total_diff])


def isRun(filename, team_id):
    with open(filename, mode="r", newline='') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        for row in spamreader:
            if row[0] == team_id:
                return True
    return False


def main():
    start_time = datetime.datetime.now()
    print("Start time:", start_time)
    file_list = get_production_file_list("./production")
    print(file_list)
    filename = 'result.csv'
    if not os.path.exists(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)
    not_run = []
    new_run_count = 0
    for file in file_list:
        team_id = get_team_id(file)
        print("*" * 80)
        print(">>> Team ID: %s, File:%s" % (team_id, file))
        if isRun(filename, team_id):
            print("Has ready run, skip.")
            continue
        try:
            player_path = uncompress_file(file)
        except NotFindGameClientFile:
            not_run.extend([team_id])
            print("[Error] Can not find the %s gameclient.bat file" % team_id)
            continue
        except NotSupportFileType:
            not_run.extend([team_id])
            print("[Error] do not support this type of compress file: ", os.path.basename(file))
            continue
        player_output_path = os.path.join(OUTPUT_DIR, team_id)
        if not os.path.exists(player_output_path):
            os.mkdir(player_output_path)
        new_run_count += 1
        score_list = run_games(team_id, player_path)
        write_result(filename, team_id, score_list)

    print("="*60)
    end_time = datetime.datetime.now()
    print("Start Time:\t\t", start_time)
    print("End Time:\t\t", end_time)
    print("Elapsed Time:\t", end_time - start_time)
    print("Total Number: ", len(file_list))
    print("Not run team:", not_run)
    print("New run count:", new_run_count)
    print("="*60)
    print("Finish!!")


if __name__ == '__main__':
   sys.exit(main())