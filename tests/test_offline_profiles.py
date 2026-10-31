"""Exercise productive profile admission/frame code with modeled SourceMod services."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def function(text, name):
    start = re.search(r'(?m)^(?:public )?\w+ '+name+r'\(', text).start()
    op = text.index('{', start)
    depth, i = 1, op+1
    while depth:
        depth += (text[i] == '{') - (text[i] == '}')
        i += 1
    return text[start:i].replace('public ', '').replace('any ', 'int ')


class OfflineProfiles(unittest.TestCase):
    def test_local_identity_keeps_deferred_summary_and_immutable_account(self):
        productive = (ROOT/'addons/sourcemod/scripting/l4d2_familyshare_core/profiles.sp').read_text()
        code = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
using Handle = int;
struct FamilyShareProfileRequest {
    int accountId=0;
    Handle httpRequest=0;
    float deadline=0;
    char steamId64[32]={}, personaName[64]={}, profileUrl[256]={};
};
struct Requests {
    std::map<int,FamilyShareProfileRequest> items;
    void SetArray(const char*key,FamilyShareProfileRequest r,int) { items[std::stoi(key)]=r; }
} storage;
Requests &g_smProfileRequests=storage;
bool g_bProfilesStopping=false;
int g_iNextProfileRequest=1, argument=0, frameId=0, summaries=0;
int GetNativeCell(int) { return argument; }
float GetEngineTime() { return 100; }
void FamilyShare_AccountIdToSteamId64(int account,char*out,int size) {
    snprintf(out,size,"%llu",76561197960265728ULL+uint32_t(account));
}
int FamilyShare_FindConnectedAccount(int) { return 0; }
void GetClientName(int,char*out,int size) { snprintf(out,size,"name"); }
void FormatEx(char*out,int size,const char*fmt,const char*value) { snprintf(out,size,fmt,value); }
void IntToString(int value,char*out,int size) { snprintf(out,size,"%d",value); }
void Frame_StartProfileRequest(int);
void RequestFrame(void(*callback)(int),int id) { assert(callback==Frame_StartProfileRequest);frameId=id; }
bool Profiles_Get(int id,FamilyShareProfileRequest&r) {
    if (!storage.items.count(id)) return false;
    r=storage.items[id];return true;
}
void Profiles_StartSummary(int id,FamilyShareProfileRequest r) {
    assert(id==frameId);assert(r.accountId==argument);
    assert(std::string(r.steamId64)=="76561202255233023");
    assert(std::string(r.profileUrl)=="https://steamcommunity.com/profiles/76561202255233023");
    assert(r.deadline==125);summaries++;
}
'''
        code += function(productive, 'Native_RequestProfile')+'\n'+function(productive, 'Frame_StartProfileRequest')
        code += r'''
int main() {
    argument=0;assert(Native_RequestProfile(0,1)==0);assert(frameId==0);
    argument=-1;int id=Native_RequestProfile(0,1);assert(id>0);assert(frameId==id);
    assert(summaries==0);Frame_StartProfileRequest(id);assert(summaries==1);
    Frame_StartProfileRequest(id+1);assert(summaries==1);
    g_bProfilesStopping=true;assert(Native_RequestProfile(0,1)==0);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source, binary = Path(tmp)/'profile.cpp', Path(tmp)/'profile'
            source.write_text(code)
            result = subprocess.run([shutil.which('clang++') or 'g++', '-std=c++17', '-fsanitize=address,undefined', '-g', str(source), '-o', str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            subprocess.run([str(binary)], check=True)
