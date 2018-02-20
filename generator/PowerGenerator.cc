#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <sstream>

std::string definitionFile;

class InputParser
{
    std::istream& stream;
    std::string str;
    bool good;
    int lineno, oldLineno;

    void read();
public:
    InputParser(std::istream& in);

    bool haveMore() { return good; }
    std::string get();
    std::string peek() { return str; }
    std::string getBlock();
    int getLineno() { return oldLineno; }

    template<typename... Args>
    void error(Args... args)
    {
        std::cerr << definitionFile << ":" << oldLineno << ": ";
        (std::cerr << ... << args);
        std::cerr << std::endl;
        std::exit(1);
    }
};

InputParser::InputParser(std::istream& in)
    : stream(in), good(true), oldLineno(1), lineno(0)
{
    read();
}

void InputParser::read()
{
    good = false;
    while(stream)
    {
        ++lineno;
        std::getline(stream, str);

        std::string::const_iterator p = 
            std::find_if(str.begin(), str.end(), [](char c) { return !std::isspace(c); });

        if(p == str.end())
            continue;
        if(str.substr(p - str.begin(), 2) == "//")
            continue;

        good = true;
        break;
    }
}

std::string InputParser::get()
{
    if(!good)
        error("unexpected EOF");
    oldLineno = lineno;
    auto s = str;
    read();
    return s;
}

std::string InputParser::getBlock()
{
    if(!good)
        error("unexpected EOF, block expected");
    oldLineno = lineno;
    std::ostringstream out;
    out << str << std::endl;
    str = "";

    while(stream)
    {
        ++lineno;
        std::getline(stream, str);

        if(!str.empty() && !std::isspace(str[0]) && str.substr(0,2) != "//")
            break;

        out << str << std::endl;
    }
    good = !str.empty();

    return out.str();
}

class SeparatedListHelper
{
    std::ostream& out;
    std::string sep;
    bool first;
public:
    SeparatedListHelper(std::ostream& out, std::string sep)
        : out(out), sep(sep), first(true)
    {}
    std::ostream& next()
    {
        if(first)
            first = false;
        else
            out << sep;
        return out;
    }

    bool empty() const { return first; }
};

struct Field
{
    int from, to;
    std::string name;
    uint32_t value;
    bool match;

    Field(int from, int to, std::string str);

    void generateCondition(SeparatedListHelper& out);
    void generateDecl(std::ostream& out);
};


Field::Field(int from, int to, std::string str)
    : from(from), to(to)
{
    value = 0;
    match = true;

    for(char c : str)
    {
        if(c == '1')
            value = 2 * value + 1;
        else if(c == '0')
            value = 2 * value;
        else
            match = false;
    }
    if(!match)
    {
        value = 0;
        name = str;
    }
}

void Field::generateCondition(SeparatedListHelper& out)
{
    if(match)
    {
        uint32_t mask = 2*(1 << (31-from)) - (1 << (32-to));
        uint32_t svalue = value << (32-to);
        out.next() << "(insn & 0x" << std::hex << mask << ") == 0x" << std::hex << svalue;
    }
}
void Field::generateDecl(std::ostream& out)
{
    if(!name.empty())
    {
        if(match)
        {
            out << "    const uint32_t " << name << " = 0x" << std::hex << value << ";" << std::endl;
        }
        else
        {
            out << "    uint32_t " << name << " = (insn >> " << std::dec << (32-to) << ") & 0x" << std::hex << 
                (1 << (to-from)) - 1 << ";" << std::endl;
        }
    }
}


class InstructionInfo
{
    std::string name;
    std::vector<Field> fields;
    std::string code;
    int lineno;
public:
    InstructionInfo(InputParser& in);

    void generateElseIf(std::ostream& out);
    void generateInterpElseIf(std::ostream& out);
    void generateDisassElseIf(std::ostream& out);
};

InstructionInfo::InstructionInfo(InputParser& in)
{
    name = in.get();

    std::string format = in.get();
    std::string bits = in.get();

    std::vector<std::string> fieldLabels;
    std::vector<int> fieldBits;

    std::istringstream fin(format), bin(bits);
    while(fin && bin)
    {
        std::string s;
        int b;
        fin >> s >> std::ws;
        bin >> b >> std::ws;
        if(!fin && !bin)
            break;
        if(!fin || !bin)
            in.error("can't parse instruction layout at field ", fieldLabels.size() + 1);
        if(fin.tellg() != bin.tellg())
            in.error("instruction layout misaligned at field ", fieldLabels.size() + 1);
        fieldLabels.push_back(s);
        fieldBits.push_back(b);
    }
    fieldBits.push_back(32);
    for(int i = 0; i < fieldLabels.size(); i++)
        fields.emplace_back(fieldBits[i], fieldBits[i+1], fieldLabels[i]);

    if(in.peek() == "===")
    {
        code = "unimplemented(\"" + name + "\")\n;";
    }
    else
    {
        code = in.getBlock();
    }
    lineno = in.getLineno();

    if(in.get() != "===")
        in.error("'===' expected at end of instruction");
}

void InstructionInfo::generateElseIf(std::ostream& out)
{
    out << "else if(";
    SeparatedListHelper slh(out, " && ");
    for(auto f : fields)
        f.generateCondition(slh);
    out << ")\n";
    out << "{\n";
    for(auto f : fields)
        f.generateDecl(out);
    out << "\n";
    out << "#line " << std::dec << lineno << " \"" << definitionFile << "\"\n";
}
void InstructionInfo::generateInterpElseIf(std::ostream& out)
{
    generateElseIf(out);
    out << code;
    out << "}\n";
}

void InstructionInfo::generateDisassElseIf(std::ostream& out)
{
    generateElseIf(out);
    out << "std::clog << std::left << std::setfill(' ') << std::setw(10) << \"" << name << "\"";
    bool first = true;
    for(auto f : fields)
    {
        if(f.name.empty())
            continue;
        
        if(first)
            first = false;
        else
            out << " << \", \"";
        out << " << std::dec << " << f.name;
    }
    out << " << std::endl;\n";
    out << "}\n";
}

int main(int argc, char *argv[])
{
    if(argc != 2)
        return 1;
    definitionFile = argv[1];
    std::ifstream stream(definitionFile);
    std::vector<InstructionInfo> insns;
    InputParser in(stream);
    while(in.haveMore())
    {
        insns.emplace_back(in);
    }

    {
        std::ofstream out("generated.interpret1.h");
        for(auto insn : insns)
            insn.generateInterpElseIf(out);
    }
    {
        std::ofstream out("generated.disass.h");
        for(auto insn : insns)
            insn.generateDisassElseIf(out);
    }

    return 0;
}