#include "jmsg.h"
#include "jmsg_util.h"
#include "jmsg_code_writer.h"
#include <fstream>

enum GenerateType {
    kGenerateTypeBinary = 0,
    kGenerateTypeJson   = 1,
    kGenerateTypeBoth   = 2,
};

GenerateType s_generateType = kGenerateTypeBinary;

static bool isGenerateJson()
{
    return s_generateType == kGenerateTypeBoth || s_generateType == kGenerateTypeJson;
}

static bool isGenerateBinary()
{
    return s_generateType == kGenerateTypeBoth || s_generateType == kGenerateTypeBinary;
}

void writeTypeDeclare(JMsgType* type, JMSGCodeWriter& writer)
{
    writer.writeLine("class %s;", type->m_typeName.c_str());
}

void writeClassDeclare(JMsgType* type, JMSGCodeWriter& writer)
{
    writer.writeLine("class %s : public IJMsgEncodeable", type->m_typeName.c_str());
    writer.writeLine("{");
    writer.writeLine("public:");
    writer.addIndent();
    writer.writeLine("%s();", type->m_typeName.c_str());
    for (JMsgField* field : type->m_vecFields) {
        std::string typeName = field->m_type;
        if (typeName == "int64") {
            typeName = "int64_t";
        } else
        if (typeName == "string") {
            typeName = "std::string";
        } else
        if (typeName == "wstring") {
            typeName = "std::wstring";
        }

        if (!field->m_isArray) {
            writer.writeLine("%s %s;", typeName.c_str(), field->m_name.c_str());
        } else {
            writer.writeLine("std::vector<%s> %s;", typeName.c_str(), field->m_name.c_str());
        }
    }

    if (isGenerateBinary()) {
        writer.writeLine("virtual void encode(JMsgWriter* writer);");
        writer.writeLine("virtual bool decode(JMsgReader* reader);");
    } else {
        writer.writeLine("virtual void encode(JMsgWriter* writer) {}");
        writer.writeLine("virtual bool decode(JMsgReader* reader) { return false; }");
    }

    if (isGenerateJson()) {
        writer.writeLine("virtual void encodeJson(Json::Value& val);");
        writer.writeLine("virtual bool decodeJson(Json::Value& val);");
    } else {
        writer.writeLine("virtual void encodeJson(Json::Value& val) {}");
        writer.writeLine("virtual bool decodeJson(Json::Value& val) { return false; }");
    }

    writer.removeIndent();
    writer.writeLine("};");
    writer.writeLine("");
}

void writeClassImplement(JMsgType* type, JMSGCodeWriter& writer)
{
    writer.writeLine("%s::%s() {", type->m_typeName.c_str(), type->m_typeName.c_str());
    writer.addIndent();
    writer.writeLine("m_msgId = %d;", type->m_id);

    for (JMsgField* field : type->m_vecFields) {
        if (field->m_isArray) {
            continue;
        }

        if (field->m_type == "double" || field->m_type == "int" || field->m_type == "int64") {
            writer.writeLine("%s = 0;", field->m_name.c_str());
        } else
        if (field->m_type == "bool") {
            writer.writeLine("%s = false;", field->m_name.c_str());
        }
    }
    writer.removeIndent();
    writer.writeLine("}");
    writer.writeLine("");

    if (isGenerateBinary()) {
        // binary encode/decode
        // start decode
        writer.writeLine("static bool on%sDecode(JMsgProto* proto, JMsgField* field, JMsgReader* reader, void* args) {", type->m_typeName.c_str());
        writer.addIndent();
        if (type->m_vecFields.size() != 0) {
            writer.writeLine("bool isSuccess = false;");
            writer.writeLine("%s* value = (%s*)args;", type->m_typeName.c_str(),  type->m_typeName.c_str());
            writer.writeLine("switch (field->m_id) {");
            for (JMsgField* field : type->m_vecFields) {
                printf("generating field:%s\n", field->m_name.c_str());
                writer.writeLine("case %d: {", field->m_id);
                writer.addIndent();

                if (field->m_isArray) {
                    writer.writeLine("int arrayLen = reader->readArrayLength(isSuccess);");
                    writer.writeLine("if (!isSuccess) break;");
                    writer.writeLine("for (int i = 0; i < arrayLen; i++) {");
                    writer.addIndent();
                    if (field->m_type == "string") {
                        writer.writeLine("value->%s.emplace_back(reader->readString(isSuccess));", field->m_name.c_str());
                    } else
                    if (field->m_type == "wstring") {
                        writer.writeLine("value->%s.emplace_back(reader->readWString(isSuccess));", field->m_name.c_str());
                    } else
                    if (field->m_type == "int") {
                        writer.writeLine("value->%s.emplace_back(reader->readInt(isSuccess));", field->m_name.c_str());
                    } else
                    if (field->m_type == "int64") {
                        writer.writeLine("value->%s.emplace_back(reader->readInt64(isSuccess));", field->m_name.c_str());
                    } else
                    if (field->m_type == "double"){
                        writer.writeLine("value->%s.emplace_back(reader->readDouble(isSuccess));", field->m_name.c_str());
                    } else
                    if (field->m_type == "bool") {
                        writer.writeLine("value->%s.emplace_back(reader->readBool(isSuccess));", field->m_name.c_str());
                    }
                    else
                    if (field->m_typeId != 0) {
                        writer.writeLine("%s item;", field->m_type.c_str());
                        writer.writeLine("isSuccess = item.decode(reader);");
                        writer.writeLine("if (!isSuccess) break;");
                        writer.writeLine("value->%s.emplace_back(item);", field->m_name.c_str());
                    }
                    writer.removeIndent();
                    writer.writeLine("}");
                } else
                if (field->m_type == "string") {
                    writer.writeLine("value->%s = reader->readString(isSuccess);", field->m_name.c_str());
                } else
                if (field->m_type == "wstring") {
                    writer.writeLine("value->%s = reader->readWString(isSuccess);", field->m_name.c_str());
                } else
                if (field->m_type == "int") {
                    writer.writeLine("value->%s = reader->readInt(isSuccess);", field->m_name.c_str());
                } else
                if (field->m_type == "int64") {
                    writer.writeLine("value->%s = reader->readInt64(isSuccess);", field->m_name.c_str());
                } else
                if (field->m_type == "double") {
                    writer.writeLine("value->%s = reader->readDouble(isSuccess);", field->m_name.c_str());
                } else
                if (field->m_type == "bool") {
                    writer.writeLine("value->%s = reader->readBool(isSuccess);", field->m_name.c_str());
                }
                else
                if (field->m_typeId != 0) {
                    writer.writeLine("isSuccess = value->%s.decode(reader);", field->m_name.c_str());
                }
                writer.writeLine("break;");
                writer.removeIndent();
                writer.writeLine("}");
            }
            writer.removeIndent();
            writer.addIndent();
            writer.writeLine("default:");
            writer.addIndent();
            writer.writeLine("break;");
            writer.removeIndent();
            writer.writeLine("}");
            writer.writeLine("return isSuccess;");
        } else {
            writer.writeLine("return true;");
        }
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");

        // start encode
        writer.writeLine("static bool on%sEncode(JMsgProto* proto, JMsgField* field, JMsgWriter* writer, void* args) {", type->m_typeName.c_str());
        writer.addIndent();

        if (type->m_vecFields.size() != 0) {
            writer.writeLine("%s* value = (%s*)args;", type->m_typeName.c_str(),  type->m_typeName.c_str());
            writer.writeLine("switch (field->m_id) {");
            for (JMsgField* field : type->m_vecFields) {
                printf("generating field:%s\n", field->m_name.c_str());
                writer.writeLine("case %d: {", field->m_id);
                writer.addIndent();
                if (field->m_isArray) {
                    writer.writeLine("int arrayLen = (int)value->%s.size();", field->m_name.c_str());
                    writer.writeLine("writer->writeArrayHeader(field, arrayLen);");
                    writer.writeLine("for (int i = 0; i < arrayLen; i++) {");
                    writer.addIndent();
                    if (field->m_type == "string") {
                        writer.writeLine("writer->writeString(value->%s[i]);", field->m_name.c_str());
                    } else
                    if (field->m_type == "wstring") {
                        writer.writeLine("writer->writeWString(value->%s[i]);", field->m_name.c_str());
                    } else
                    if (field->m_type == "int") {
                        writer.writeLine("writer->writeInt(value->%s[i]);", field->m_name.c_str());
                    } else
                    if (field->m_type == "int64") {
                        writer.writeLine("writer->writeInt64(value->%s[i]);", field->m_name.c_str());
                    }
                    else
                    if (field->m_type == "double"){
                        writer.writeLine("writer->writeDouble(value->%s[i]);", field->m_name.c_str());
                    } else
                    if (field->m_type == "bool") {
                        writer.writeLine("writer->writeBool(value->%s[i]);", field->m_name.c_str());
                    }
                    else
                    if (field->m_typeId != 0) {
                        writer.writeLine("value->%s[i].encode(writer);", field->m_name.c_str());
                    }
                    writer.removeIndent();
                    writer.writeLine("}");
                } else
                if (field->m_type == "string") {
                    writer.writeLine("writer->writeStringField(field, value->%s);", field->m_name.c_str());
                } else
                if (field->m_type == "wstring") {
                    writer.writeLine("writer->writeWStringField(field, value->%s);", field->m_name.c_str());
                } else
                if (field->m_type == "int") {
                    writer.writeLine("writer->writeIntField(field, value->%s);", field->m_name.c_str());
                } else
                if (field->m_type == "int64") {
                    writer.writeLine("writer->writeInt64Field(field, value->%s);", field->m_name.c_str());
                }
                else
                if (field->m_type == "double") {
                    writer.writeLine("writer->writeDoubleField(field, value->%s);", field->m_name.c_str());
                } else
                if (field->m_type == "bool") {
                    writer.writeLine("writer->writeBoolField(field, value->%s);", field->m_name.c_str());
                }
                else
                if (field->m_typeId != 0) {
                    writer.writeLine("writer->writeFieldHeader(field);", field->m_name.c_str());
                    writer.writeLine("value->%s.encode(writer);", field->m_name.c_str());
                }
                writer.writeLine("break;");
                writer.removeIndent();
                writer.writeLine("}");
            }
            writer.removeIndent();
            writer.addIndent();
            writer.writeLine("default:");
            writer.addIndent();
            writer.writeLine("break;");
            writer.removeIndent();
            writer.writeLine("}");
        }
        writer.writeLine("return true;");
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");
        writer.writeLine("void %s::encode(JMsgWriter* writer) {", type->m_typeName.c_str());
        writer.addIndent();
        writer.writeLine("g_proto->encode(%d, writer, on%sEncode, this);", type->m_id, type->m_typeName.c_str());
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");
        writer.writeLine("bool %s::decode(JMsgReader* reader) {", type->m_typeName.c_str());
        writer.addIndent();
        writer.writeLine("return g_proto->decode(reader, on%sDecode, this) == %d;", type->m_typeName.c_str(), type->m_id);
        writer.removeIndent();
        writer.writeLine("}");
    }

    if (isGenerateJson()) {
        // json encode/decode
        // start decode
        writer.writeLine("");
        writer.writeLine("static bool on%sDecodeJson(JMsgProto* proto, JMsgField* field, Json::Value& jsonValue, void* args) {", type->m_typeName.c_str());
        writer.addIndent();
        if (type->m_vecFields.size() != 0) {
            writer.writeLine("bool isSuccess = false;");
            writer.writeLine("%s* value = (%s*)args;", type->m_typeName.c_str(),  type->m_typeName.c_str());
            writer.writeLine("switch(field->m_id) {");
            for (JMsgField* field : type->m_vecFields) {
                printf("generating field:%s\n", field->m_name.c_str());
                writer.writeLine("case %d: {", field->m_id);
                writer.addIndent();

                if (field->m_isArray) {
                    writer.writeLine("for (size_t i = 0; i < jsonValue.size(); i++) {");
                    writer.addIndent();
                    if (field->m_type == "string") {
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isString() ? jsonValue[(int)i].asString() : \"\");", field->m_name.c_str());
                    } else
                    if (field->m_type == "wstring") {
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isString() ? jsonValue[(int)i].asString() : \"\");", field->m_name.c_str());
                    } else
                    if (field->m_type == "int") {
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isInt() ? jsonValue[(int)i].asInt() : 0);", field->m_name.c_str());
                    } else
                    if (field->m_type == "int64") {
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isInt64() ? jsonValue[(int)i].asInt64() : 0);", field->m_name.c_str());
                    } else
                    if (field->m_type == "double"){
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isDouble() ? jsonValue[(int)i].asDouble(): 0);", field->m_name.c_str());
                    } else
                    if (field->m_type == "bool") {
                        writer.writeLine("value->%s.emplace_back(jsonValue[(int)i].isBool() ? jsonValue[(int)i].asBool() : false);", field->m_name.c_str());
                    }
                    else
                    if (field->m_typeId != 0) {
                        writer.writeLine("%s item;", field->m_type.c_str());
                        writer.writeLine("isSuccess = item.decodeJson(jsonValue[(int)i]);");
                        writer.writeLine("if (!isSuccess) break;");
                        writer.writeLine("value->%s.emplace_back(item);", field->m_name.c_str());
                    }
                    writer.removeIndent();
                    writer.writeLine("}");
                } else
                if (field->m_type == "string") {
                    writer.writeLine("value->%s = jsonValue.isString() ? jsonValue.asString() : \"\";", field->m_name.c_str());
                } else
                if (field->m_type == "wstring") {
                    writer.writeLine("value->%s = jsonValue.isString() ? jsonValue.asString() : \"\";", field->m_name.c_str());
                } else
                if (field->m_type == "int") {
                    writer.writeLine("value->%s = jsonValue.isInt() ? jsonValue.asInt() : 0;", field->m_name.c_str());
                } else
                if (field->m_type == "int64") {
                    writer.writeLine("value->%s = jsonValue.isInt64() ? jsonValue.asInt64() : 0;", field->m_name.c_str());
                }
                else
                if (field->m_type == "double") {
                    writer.writeLine("value->%s = jsonValue.isDouble() ? jsonValue.asDouble() : 0;", field->m_name.c_str());
                } else
                if (field->m_type == "bool") {
                    writer.writeLine("value->%s = jsonValue.isBool() ? jsonValue.asBool() : false;", field->m_name.c_str());
                }
                else
                if (field->m_typeId != 0) {
                    writer.writeLine("isSuccess = value->%s.decodeJson(jsonValue);", field->m_name.c_str());
                }
                writer.writeLine("break;");
                writer.removeIndent();
                writer.writeLine("}");
            }
            writer.removeIndent();
            writer.addIndent();
            writer.writeLine("default:");
            writer.addIndent();
            writer.writeLine("break;");
            writer.removeIndent();
            writer.writeLine("}");
            writer.writeLine("return isSuccess;");
        }
        else {
            writer.writeLine("return true;");
        }
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");

        // start encode
        writer.writeLine("static bool on%sEncodeJson(JMsgProto* proto, JMsgField* field, Json::Value& jsonValue, void* args) {", type->m_typeName.c_str());
        writer.addIndent();

        if (type->m_vecFields.size() != 0) {
            writer.writeLine("%s* value = (%s*)args;", type->m_typeName.c_str(),  type->m_typeName.c_str());
            writer.writeLine("switch (field->m_id) {");

            for (JMsgField* field : type->m_vecFields) {
                printf("generating field:%s\n", field->m_name.c_str());
                writer.writeLine("case %d: {", field->m_id);
                writer.addIndent();

                if (field->m_isArray) {
                    writer.writeLine("Json::Value arrayValue;");
                    writer.writeLine("for (size_t i = 0; i < value->%s.size(); i++) {", field->m_name.c_str());
                    writer.addIndent();
                    if (field->m_type == "string" || field->m_type == "int" || field->m_type == "int64" || field->m_type == "bool" || field->m_type == "double") {
                        writer.writeLine("arrayValue.append(value->%s[i]);", field->m_name.c_str());
                    } else
                    if (field->m_typeId != 0) {
                        writer.writeLine("Json::Value itemValue;");
                        writer.writeLine("value->%s[i].encodeJson(itemValue);", field->m_name.c_str());
                        writer.writeLine("arrayValue.append(itemValue);");
                    }
                    writer.removeIndent();
                    writer.writeLine("}");
                    // writer.writeLine("if (arrayValue.size()) {");
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = arrayValue;", field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_type == "string") {
                    // writer.writeLine("if (!value->%s.empty()) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_type == "wstring") {
                    // writer.writeLine("if (!value->%s.empty()) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                }
                else
                if (field->m_type == "int") {
                    // writer.writeLine("if (value->%s != 0) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_type == "int64") {
                    // writer.writeLine("if (value->%s != 0) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_type == "double") {
                    // writer.writeLine("if (value->%s != 0.0) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_type == "bool") {
                    // writer.writeLine("if (value->%s != false) {", field->m_name.c_str());
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = value->%s;", field->m_name.c_str(), field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                } else
                if (field->m_typeId != 0) {
                    writer.writeLine("Json::Value itemValue;");
                    writer.writeLine("value->%s.encodeJson(itemValue);", field->m_name.c_str());
                    // writer.writeLine("if (!itemValue.empty()) {");
                    // writer.addIndent();
                    writer.writeLine("jsonValue[\"%s\"] = itemValue;", field->m_name.c_str());
                    // writer.removeIndent();
                    // writer.writeLine("}");
                }
                writer.writeLine("break;");
                writer.removeIndent();
                writer.writeLine("}");
            }
            writer.removeIndent();
            writer.addIndent();
            writer.writeLine("default:");
            writer.addIndent();
            writer.writeLine("break;");
            writer.removeIndent();
            writer.writeLine("}");
        }
        writer.writeLine("return true;");
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");
        writer.writeLine("void %s::encodeJson(Json::Value& writer) {", type->m_typeName.c_str());
        writer.addIndent();
        writer.writeLine("g_proto->encodeJson(%d, writer, on%sEncodeJson, this);", type->m_id, type->m_typeName.c_str());
        writer.removeIndent();
        writer.writeLine("}");
        writer.writeLine("");
        writer.writeLine("bool %s::decodeJson( Json::Value& reader) {", type->m_typeName.c_str());
        writer.addIndent();
        writer.writeLine("return g_proto->decodeJson(%d, reader, on%sDecodeJson, this);", type->m_id, type->m_typeName.c_str());
        writer.removeIndent();
        writer.writeLine("}");
    }
}

void writeHeaderCollect(const std::string& baseDir, const std::string& headerName, JMsgProto* protos)
{
    JMSGCodeWriter writer;
    std::string filePath = baseDir + headerName + ".h";
    if (!writer.open(filePath)) {
        return;
    }

    writer.writeLine("#ifndef %s_h", headerName.c_str());
    writer.writeLine("#define %s_h", headerName.c_str());
    for (JMsgType* msgType : protos->getAllTypes()) {
        writer.writeLine("#include \"%s.h\"", msgType->m_typeName.c_str());
    }
    writer.writeLine("#endif");
}

void generateHeaderFile(std::string outputPath, std::string prefix, std::vector<JMsgType*>& types)
{
    JMSGCodeWriter headerWriter;
    std::string fileName = jMsgGetFormatString("%s%s%s", outputPath.c_str(), prefix.c_str(), ".h");//outputPath + prefix + ".h";
    if (!headerWriter.open(fileName)) {
        printf("open file %s failed\n", fileName.c_str());
        return;
    }

    headerWriter.writeLine("#ifndef %s_h",  prefix.c_str());
    headerWriter.writeLine("#define %s_h",  prefix.c_str());
    headerWriter.writeLine("#include <stdio.h>");
    headerWriter.writeLine("#include <vector>");
    headerWriter.writeLine("#include <string>");
    headerWriter.writeLine("#include \"json/value.h\"");
    headerWriter.writeLine("#include \"json/reader.h\"");
    headerWriter.writeLine("#include \"jmsg_encodeable.h\"");
    headerWriter.writeLine("#pragma warning(disable: 4100)");
    headerWriter.writeLine("class JMsgWriter;");
    headerWriter.writeLine("class JMsgReader;");
    headerWriter.writeLine("class JMsgProto;");
    headerWriter.writeLine("enum %sTypeIds {", prefix.c_str());
    headerWriter.addIndent();
    for (size_t i = 0; i < types.size(); i++) {
        headerWriter.writeLine("k%s = %d,", types[i]->m_typeName.c_str(), types[i]->m_id);
    }
    headerWriter.removeIndent();
    headerWriter.writeLine("};");
    headerWriter.writeLine("JMsgProto* %sCreateProto(bool fixFieldLen = true);", prefix.c_str());
    headerWriter.writeLine("void %sInit();", prefix.c_str());
    headerWriter.writeLine("void %sFini();", prefix.c_str());
    headerWriter.writeLine("JMsgProto* %sGetProto();", prefix.c_str());
    for (size_t i = 0; i < types.size(); i++) {
        writeClassDeclare(types[i], headerWriter);
    }
    headerWriter.writeLine("#pragma warning(default: 4100)");
    headerWriter.writeLine("#endif");
}

void generateDeclareFile(std::string outputPath, std::string prefix, std::vector<JMsgType*>& types)
{
    JMSGCodeWriter declareWriter;
    std::string path = outputPath + prefix + "Declare.h";
    declareWriter.open(path);
    declareWriter.writeLine("#ifndef %sDeclare_h",  prefix.c_str());
    declareWriter.writeLine("#define %sDeclare_h",  prefix.c_str());
    for (size_t i = 0; i < types.size(); i++) {
        writeTypeDeclare(types[i], declareWriter);
    }
    declareWriter.writeLine("#endif");
}

void generateCppFile(std::string outputPath, std::string prefix, std::vector<JMsgType*>& types, std::string& content)
{
    JMSGCodeWriter cppWriter;
    std::string path = outputPath + prefix + ".cpp";
    cppWriter.open(path);
    cppWriter.writeLine("#include \"%s.h\"", prefix.c_str());
    cppWriter.writeLine("#include \"jmsg.h\"");
    cppWriter.writeLine("#pragma warning(disable: 4100)");
    cppWriter.write("\n");
    cppWriter.write("static const unsigned char s_protoString []= {");
    for (size_t i = 0; i < content.size(); i++) {
        if (i % 10 == 0) {
            cppWriter.write("\n    ");
        }
        cppWriter.write("0x%02x,", (unsigned char)content[i]);
    }
    cppWriter.write("0x00");
    cppWriter.write("\n");
    cppWriter.writeLine("};");
    cppWriter.writeLine("");
    cppWriter.writeLine("JMsgProto* %sCreateProto(bool fixFieldLen) { return JMsgProto::createProto((char*)s_protoString, fixFieldLen); }", prefix.c_str());
    cppWriter.writeLine("static JMsgProto* g_proto = nullptr;");
    cppWriter.writeLine("void %sInit(){", prefix.c_str());
    cppWriter.addIndent();
    cppWriter.writeLine("g_proto = %sCreateProto();", prefix.c_str());
    cppWriter.removeIndent();
    cppWriter.writeLine("}");
    cppWriter.writeLine("void %sFini(){", prefix.c_str());
    cppWriter.addIndent();
    cppWriter.writeLine("if (g_proto) {");
    cppWriter.addIndent();
    cppWriter.writeLine("delete g_proto;");
    cppWriter.writeLine("g_proto = nullptr;");
    cppWriter.removeIndent();
    cppWriter.writeLine("}");
    cppWriter.removeIndent();
    cppWriter.writeLine("}");
    cppWriter.writeLine("");
    cppWriter.writeLine("JMsgProto* %sGetProto() { return g_proto; }", prefix.c_str());
    cppWriter.writeLine("");
    for (size_t i = 0; i < types.size(); i++) {
        writeClassImplement(types[i], cppWriter);
    }
    cppWriter.writeLine("#pragma warning(default: 4100)");
}

std::string getFileContext(const char* filename) {
    std::string content;
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    if (fin) {
        fin.seekg(0, std::ios::end);
        content.resize(fin.tellg());
        fin.seekg(0, std::ios::beg);
        fin.read(&content[0], content.size());
        fin.close();
    }
    return content;
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        printf("useage:jmsg_generator ${config_file_name} $(output_path) $(all_header_name) [$GenerateType], argc=%d\n", argc);
        printf("generate type=binary | json | go | both\n");
        return 0;
    }

    if (argc >= 5) {
        std::string generateType = argv[4];
        if (generateType == "binary") {
            s_generateType = kGenerateTypeBinary;
        } else
        if (generateType == "json") {
            s_generateType = kGenerateTypeJson;
        } else
        if (generateType == "both") {
            s_generateType = kGenerateTypeBoth;
        } else {
            printf("error generate type:must be one of the below,\"binary\",\"json\",\"both\"\n");
            return 0;
        }
    }

    std::string content = getFileContext(argv[1]);
    printf("creating proto\n");
    JMsgProto* proto = JMsgProto::createProto(content);
    if (!proto) {
        printf("create proto failed\n");
        return 0;
    }
    printf("creating proto success\n");

    std::string outputPath = argv[2];
    std::string prefix = argv[3];
    std::vector<JMsgType*>& types = proto->getAllTypes();

    generateHeaderFile(outputPath, prefix, types);
    generateDeclareFile(outputPath, prefix, types);
    generateCppFile(outputPath, prefix, types, content);
    return 0;
}
