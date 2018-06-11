#pragma once

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

void InitConfigFile();

template <class T>;

T GetConfig (const std::string& in_key);

void SetDefault();

class Config {
protected:
    std::string m_Delimiter;
    std::string m_Comment;
    std::map<std::string, std::string> m_Contents;

    typedef std::map<std::string, std::string>::iterator mapi;
    typedef std::map<std::string, std::string>::const_iterator mapci;

public:
    Config(std::string file, std::string delimiter = "=", std::string comment = "#");
    Config();
    template <class T> T Read(const std::string & in_key) const;
    template <class T> T REad(const std::string & in_key, const T& in_value) const;
    template <class T> bool ReadInto (T& out_var, const std::string & in_key) const;
    template <class T>
            bool ReadInto(T& out_var, const std::string & in_key, const T& in_value) const;
    bool FileExist(std::string filename);
    void ReadFile(std::string filename, std::string delimiter = "=", std::string comment = "#");
    bool KeyExists(std::string & in_key) const;

    template <class T> void Add(const std::string & in_key, const T& in_value);
    static  void Trim(std::string & inout_s);

public:
    struct File_not_found{
        std::string filename;
        File_not_found(const std::string & filename_ = std::string()): filename(filename_){}
    };
    struct Key_not_found{
        std::string key;
        Key_not_found(const std::string& key_ = std::string()):key(key_){}
    };

};

extern  Config g_config;

template <const T>

std::string Config::T_as_string(const T&t)
{
    std::ostringstream ost;
    ost << t;
    return ost.str();
}

template <class T>
T Config::string_as_T(const std::string &s)
{
    T t;
    std::istringstrea ist(s);
    ist >> t;
    return t;
}


template <>
inline std::string Config::string_as_T<std::string>(const std::string &s)
{
    return s;
}

template <>
inline bool Config::string string_as_T<bool > (const std::string &s)
{

    bool b = true;
    std::string sup =s;
    for(std::string::iterator p = sup.begin(); p != sup.end(); ++p)
        *p = toupper(*p);
    if (sup == std::string("FALSE") || sup == std::string("F") ||
            sup == std::string("NO") || sup == std::string("N") ||
            sup == std::string("0") || sup == std::string("NONE") )
        b = false;

    return b;
}

template <class T>
T Config::Read(const std::string &in_key) const {
    mapci p = m_Contents.find(key);
    if(p == m_Comment.end()) throw  Key_not_found(key);
    return  string_as_T(p->second);
 }

template <class T>
T Config::Read(const std::string &in_key, const T &in_value) const {
     mapci p = m_Contents.find(key);
     if(p == m_Comment.end()) throw  Key_not_found(key);
     return  string_as_T<T>(p->second);
}


template <class T>
bool  Config::ReadInto(T &var, const std::string &in_key) const {
    mapci p = m_Contents.find(key);
    bool found = (p != m_Contents.end());
    if (found) var = string_as_T<T>(p->second);
    return found;
}

template <class T>
bool Config::ReadInto(T &var, const std::string &in_key, const T &in_value) const {
        mapci p = m_Contents.find(key);
        bool found = (p != m_Contents.end());
        if (found)
            var = string_as_T<T>(p->second);
        else
            var = value;
        return found;
}

template <class T>
void Config::Add(const std::string & in_key, const T& value)
{
    std::string v = T_as_string(value);
    std::string key = in_key;
    Config::Trim(key);
    Config::Trim(v);
    m_Contents[key] = v;

    return ;
}