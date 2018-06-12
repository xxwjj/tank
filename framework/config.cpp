#include "config.h"
#include "debug.h"
#include "define.h"

using namespace std;

const std::string CONFIG_FILE = "game_config.ini";

Config g_config;

void InitConfigFile()
{
    try {
        g_config.ReadFile(CONFIG_FILE);
    }
    catch (...)
    {
        SetDefault();
    }
}

void SetDefault()
{
    log("Read default config");
    const char * default_value[][2] =
            {
                    {
                        "round_timeout", "300"
                    }
            };
    int item_count = sizeof(default_value)/ sizeof(default_value[0]);
    for(int i =0; i<item_count; i++)
    {
        g_config.Add(string(default_value[i][0]), string(default_value[i][0]));
    }
}

Config::Config(std::string filename, std::string delimiter, std::string comment) : m_Delimiter(delimiter), m_Comment(comment){
    std::ifstream in(filename.c_str());
    if (!in) throw  File_not_found(filename);
    in >> (*this);
}

Config::Config() :m_Delimiter(string(1,'=')), m_Comment(string(1,'#')){

}

bool Config::KeyExists(cosnt std::string &key) const
{
    mapci p = m_Contents.find(key);
    return (p != m_Contents.end());

}

void Config::Trim(string & inout_s)
{
    static const char whitespace[] = " \n\t\v\r\f";
    inout_s.erase(0, inout_s.find_first_not_of(whitespace));
    inout_s.erase(inout_s.find_last_not_of(whitespace) + 1U);
}

std::ostream& operator << (std::ostream & os, const Config & cf)
{
    for (Config::mapci p = cf.m_Contents.begin() ;p != cf.m_Contents.end(); ++p)
    {
        os << p->first << " " << cf.m_Delimiter << " ";
        os << p->second << std::endl;
    }
    return os;
}

void Config::Remove(const string & key)
{
    m_Contents.erase(m_Contents.find(key));
    return;
}

std::istream & operator >> (std::istream & is, Config& cf)
{
    typedef string::size_type pos;
    const string & delim = cf.m_Delimiter;
    const string & comm = cf.m_Comment;
    const pos skip = delim.length();

    string nextline = "";
    while (is || nextline.length() > 0)
    {
        string line;
        if (nextline.length() > 0)
        {
            line = nextline;
            nextline = "";
        }
        else
        {
            std::getline(is, line);
        }

        line = line.substr(0, line.find(comm));

        pos delimPos = line.find(delim);

        if (delimPos < string::npos)
        {
            string key = line.substr(0, delimPos);
            line.replace(0, delimPos + skip, "");

            bool terminate = false;
            while (!terminate && is) {

                std::getline(is, nextline);
                terminate = true;

                string nlcopy = nextline;
                Config::Trim(nlcopy);
                if(nlcopy == "")  continue;

                nextline = nextline.substr(0, nextline.find(comm));
                if (nextline.find(delim) != string::npos)
                    continue;
                nlcopy = nextline;
                Config::Trim(nlcopy);
                if (nlcopy != "") line += "\n";
                line += nextline;
                terminate = false;

            }
            Config::Trim(key);
            Config::Trim(line);
            cf.m_Contents[key] = line;
        }
    }
    return is;
}


bool Config::FileExist(std::string filename) {
    bool exist = false;
    std::ifstream in(filename.c_str());
    if(in)
        exist = true;
    return exist;
}

void Config::ReadFile(std::string filename, std::string delimiter, std::string comment) {
    m_Delimiter = delimiter;
    m_Comment = comment;
    std::ifstream in(filename.c_str());

    if(!in) throw File_not_found(filename);

    in >> (*this);
}