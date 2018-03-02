#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <set>

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
    : stream(in), good(true), lineno(0), oldLineno(1)
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
    
    int bitsize() { return to - from; }
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
            out << "    const uint32_t " << name << " [[maybe_unused]] = 0x" << std::hex << value << ";" << std::endl;
        }
        else
        {
            out << "    uint32_t " << name << " [[maybe_unused]] = (insn >> " << std::dec << (32-to) << ") & 0x" << std::hex << 
                (1 << (to-from)) - 1 << ";" << std::endl;
        }
    }
}

class InstructionInfo
{
public:
    std::string name;
    std::vector<Field> fields;
    std::string code;
    int lineno;
    bool branch = false;
public:
    InstructionInfo(InputParser& in);

    void generateElseIf(std::ostream& out);
    void generateInterpElseIf(std::ostream& out);
    void generateDisassElseIf(std::ostream& out);

    void layoutOperands();

    void generateOperandStruct(std::ostream& out);

    void generateTranslateElseIf(std::ostream& out, int index);
    void generateOpcodeLabel(std::ostream& out);
    void generateInterp2(std::ostream& out);
    
    
    void expand(std::vector<InstructionInfo>& out);
    void joinRanges();
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
        fin >> s;
        bin >> b;
        if(!fin && !bin)
            break;
        if(!fin || !bin)
            in.error("can't parse instruction layout at field ", fieldLabels.size() + 1);
        fin >> std::ws;
        bin >> std::ws;
        if(fin.tellg() != bin.tellg())
            in.error("instruction layout misaligned at field ", fieldLabels.size() + 1);
        
        fieldLabels.push_back(s);
        fieldBits.push_back(b);
    }
    fieldBits.push_back(32);
    for(int i = 0; i < fieldLabels.size(); i++)
        fields.emplace_back(fieldBits[i], fieldBits[i+1], fieldLabels[i]);

    for(;;)
    {
        std::string command = in.peek();

        if(code.empty() && !command.empty() && std::isspace(command[0]))
        {
            code = in.getBlock();
            lineno = in.getLineno();
        }
        else
        {
            command = in.get();

            if(command.empty() || command == "===")
                break;
            else if(command == "branch")
                branch = true;
        }
    }

    if(code.empty())
    {
        code = "unimplemented(\"" + name + "\")\n;";
        lineno = in.getLineno();
    }
}

void InstructionInfo::joinRanges()
{
    if(!fields.size())
        return;
    
    std::vector<Field> newFields;
    newFields.push_back(fields[0]);
    for(int i = 1, n = fields.size(); i < n; i++)
    {
        Field& f1 = newFields.back();
        Field& f2 = fields[i];
        
        if(f1.match && f2.match)
        {
            if(f1.to == f2.from)
            {
                f1.value <<= f2.to - f2.from;
                f1.value |= f2.value;
                continue;
            }
        }
        
        newFields.push_back(f2);
    }
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
}
void InstructionInfo::generateInterpElseIf(std::ostream& out)
{
    generateElseIf(out);
    out << "#line " << std::dec << lineno << " \"" << definitionFile << "\"\n";
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

void InstructionInfo::generateOperandStruct(std::ostream& out)
{
    out << "struct Opcode_" << name << "\n";
    out << "{\n";
    out << "    TranslatedOpcode opcode;\n";

    auto sortedFields = fields;
    std::sort(sortedFields.begin(), sortedFields.end(), [](const Field& a, const Field& b) {
        return (a.to - a.from) > (b.to - b.from);
    });
    if(branch)
        out << "    uint32_t CIA;\n";

    for(const Field& f : sortedFields)
    {
        if(f.name.empty() || f.match)
            continue;
        out << "    ";
        if(f.to-f.from > 16)
            out << "uint32_t";
        else if(f.to-f.from > 8)
            out << "uint16_t";
        else
            out << "uint8_t";
        out << " " << f.name << ";\n";
    }
        
    out << "};\n\n";
}

void InstructionInfo::generateTranslateElseIf(std::ostream& out, int idx)
{
    generateElseIf(out);
    out << "    Opcode_" << name << "& translated = "
        << "*reinterpret_cast<Opcode_" << name << "*>(allocOpcode("
        << "sizeof(Opcode_" << name << ")));\n";
    out << "    translated.opcode = makeOpcode(" << std::dec << idx << ");\n";

    for(const Field& f : fields)
    {
        if(f.name.empty() || f.match)
            continue;
        out << "    translated." << f.name << " = " << f.name << ";\n";
    }
    if(branch)
    {
        out << "    translated.CIA = addr;\n";
        out << "    break;\n";
    }
    out << "}\n";
}

void InstructionInfo::generateOpcodeLabel(std::ostream& out)
{
    out << "    &&label_" << name << ",\n";
}

void InstructionInfo::generateInterp2(std::ostream& out)
{
    out << "label_" << name << ":\n";
    out << "{\n";

    out << "    Opcode_" << name << "& translated = "
    << "*reinterpret_cast<Opcode_" << name << "*>(code);\n";
    for(const Field& f : fields)
    {
        if(f.name.empty())
            continue;
        if(f.match)
            out << "    const uint32_t " << f.name << " [[maybe_unused]] = 0x" << std::hex << f.value << ";" << std::endl;
        else
            out << "    uint32_t " << f.name << " [[maybe_unused]] = translated." << f.name << ";\n";
    }
    if(branch)
    {
        out << "    CIA = translated.CIA;\n";
        out << "    uint32_t NIA = CIA + 4;\n";
    }

    out << code;

    if(branch)
    {
        out << "    CIA = NIA;\n";
        out << "    goto loop;\n";
    }
    else
    {
        out << "    code += sizeof(translated);\n";
        out << "    goto *(*(TranslatedOpcode*)code);\n";
    }
    out << "}\n";
}

void InstructionInfo::expand(std::vector<InstructionInfo>& out)
{
    out.push_back(*this);
    return;
    std::vector<Field*> expandedFields;
    for(Field& f : fields)
    {
        if(f.match)
            continue;
        if(f.to - f.from > 1)
            continue;
        f.match = true;
        f.value = 0;
        expandedFields.push_back(&f);
    }
    
    bool carry;
    std::string saveName = name;
    
    do
    {
        carry = true;

        for(Field *f : expandedFields)
        {
            if(!carry)
                break;
            
            uint32_t mask = ((uint32_t)1U << (uint32_t)f->bitsize()) - 1U;
            f->value = (f->value + 1) & mask;
            carry = (f->value & mask) == 0;
        }
        
        std::ostringstream nstr;
        nstr << saveName << std::hex;
        for(Field *f : expandedFields)
            nstr << "_" << f->value;
        name = nstr.str();
        std::cout << name << std::endl;
        out.push_back(*this);
    } while(!carry);
    
    name = saveName;
    
    for(Field *f : expandedFields)
    {
        f->match = false;
        f->value = 0;
    }
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
        InstructionInfo(in).expand(insns);
    }

    {
        using namespace std;
        set<pair<int,int>> matchranges;
        for(auto& insn : insns)
        {
            for(auto& field : insn.fields)
            {
                if(field.match)
                    matchranges.emplace(field.from, field.to);
            }
        }
        
        for(auto p : matchranges)
            cout << p.first << ".." << p.second << std::endl;
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
    {
        std::ofstream out("generated.opcodes.h");
        for(auto& insn : insns)
        {
            insn.generateOperandStruct(out);
        }
    }
    {
        std::ofstream out("generated.translate.h");
        int idx = 0;
        for(auto insn : insns)
            insn.generateTranslateElseIf(out, idx++);
    }
    {
        std::ofstream out("generated.opcodelabels.h");
        for(auto insn : insns)
            insn.generateOpcodeLabel(out);
    }
    {
        std::ofstream out("generated.interpret2.h");
        for(auto insn : insns)
            insn.generateInterp2(out);
    }

    return 0;
}
