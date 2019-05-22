#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include "jmsg_idl_parse.h"
#include "jmsg_type.h"
#include "jmsg_field.h"
#include "jmsg_util.h"

using namespace std;
static void jMsgOrderTypes(std::unordered_map<string, JMsgType*>& mapMessages, std::vector<JMsgType*>& vecMessages);
static char* skipEmptyChars(char* data)
{
    while (jMsgIsEmptyChar(*data)) {
        data ++;
    }
    return data;
}

static char* skipComment(char* data)
{
    while (data && *data == ';') {
        data ++;
        while (!jMsgIsChangeLine(*data)) {
            data ++;
        }

        if (data) {
            data = skipEmptyChars(data);
        }
    }
    return data;
}

static char* getLeftBrace(char* data)
{
    if (*data == '{') {
        data++;
        return data;
    }
    return nullptr;
}

static char* getcColons(char* data)
{
    if (*data == ':') {
        data++;
        return data;
    }
    return nullptr;
}

static char* getCommonWord(char* data, string& word)
{
    if (!jMsgIsAlpha(*data) && !jMsgIsUnderLine(*data)) {
        return nullptr;
    }

    word.append(data, 1);
    data++;
    while (jMsgIsAlpha(*data) || jMsgIsDigit(*data) || jMsgIsUnderLine(*data)) {
        word.append(data, 1);
        data++;
    }
    return data;
}

static char* getEqual(char* data)
{
    if (*data == '=') {
        data++;
        return data;
    }
    return nullptr;
}

static char* getSquareBrackets(char* data)
{
    if (data[0] == '[' && data[1] == ']') {
        data += 2;
        return data;
    }
    return nullptr;
}

static char* getNumber(char* data, int* number)
{
    int ret = 0;
    bool found = false;
    while (jMsgIsDigit(*data)) {
        ret = ret * 10 + (*data) - '0';
        found = true;
        data++;
    }

    if (found) {
        *number = ret;
        return data;
    }
    return nullptr;
}

static char* getField(char* data, JMsgField** pField) {
    string fieldName;
    string fieldType;
    int fieldId = 0;
    JMsgField* field = nullptr;
    bool isArray = false;

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    data = getCommonWord(data, fieldName);
    if(!data) {
        return nullptr;
    }
    // printf("read field name:%s\n", fieldName.c_str());

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = getcColons(data);
    if(!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    char* squireBranceStart = getSquareBrackets(data);
    if (squireBranceStart) {
        // printf("squre bracket found\n");
        isArray = true;
        data = squireBranceStart;
    }

    data = getCommonWord(data, fieldType);
    if (!data) {
        return nullptr;
    }
    // printf("read field type:%s\n", fieldType.c_str());

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = getEqual(data);
    if (!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = getNumber(data, &fieldId);
    if (!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    field = new JMsgField;
    field->m_name = fieldName;
    field->m_type = fieldType;
    field->m_isArray = isArray;
    field->m_id = fieldId;
    *pField = field;

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    return data;
}

static char* getType(char* data, JMsgType** ppMsgType)
{
    string typeName;
    int id;

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    data = getCommonWord(data, typeName);
    if (!data) {
        return nullptr;
    }
    // printf("gettype:%s\n", typeName.c_str());

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    data = getEqual(data);
    if (!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = getNumber(data, &id);
    if (!data) {
        return nullptr;
    }
    // printf("get type id=%d\n", id);

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = getLeftBrace(data);
    if (!data) {
        return nullptr;
    }

    data = skipEmptyChars(data);
    if (!data) {
        return nullptr;
    }

    data = skipComment(data);
    if (!data) {
        return nullptr;
    }

    JMsgType* msgType = new JMsgType;
    msgType->m_typeName = typeName;
    msgType->m_id = id;
    while(*data != '}') {
        JMsgField* field = nullptr;
        data = getField(data, &field);
        if (!data) {
            break;
        }
        msgType->m_vecFields.emplace_back(field);
    }

    if (!data) {
        delete msgType;
        return nullptr;
    }

    data++;
    *ppMsgType = msgType;
    return data;
}

static bool isBasicType(const string& fieldName)
{
    if (fieldName == "string" ||
        fieldName == "wstring" ||
        fieldName == "int" ||
        fieldName == "double" ||
        fieldName == "bool" ||
        fieldName == "int64") {
        return true;
    }
    return false;
}

static bool checkMessages(std::vector<JMsgType*>& vecMessages)
{
    std::unordered_map<std::string, JMsgType*> mapTypeNames;
    std::unordered_set<int> setTypeIds;
    for (size_t i = 0; i < vecMessages.size(); i++) {
        std::string& typeName = vecMessages[i]->m_typeName;
        // printf("adding %s\n", typeName.c_str());

        if (mapTypeNames.find(typeName) == mapTypeNames.end()) {
            mapTypeNames[typeName] = vecMessages[i];
        } else {
            return false;
        }

        if (setTypeIds.find(vecMessages[i]->m_id) == setTypeIds.end()) {
            setTypeIds.insert(vecMessages[i]->m_id);
        } else {
            return false;
        }
    }

    for (size_t i = 0; i < vecMessages.size(); i++) {
        JMsgType* msgType = vecMessages[i];
        unordered_set<int> setFieldIds;
        for (size_t j = 0; j < msgType->m_vecFields.size(); j++) {
            JMsgField* field = msgType->m_vecFields[j];

            if (!isBasicType(field->m_type)) {
                unordered_map<string, JMsgType*>::iterator iter = mapTypeNames.find(field->m_type);
                if (iter == mapTypeNames.end()) {
                    return false;
                }
                else {
                    field->m_typeId =  iter->second->m_id;
                    // printf("set field typeid:%d\n", field->m_typeId);
                }
            }

            if (setFieldIds.find(field->m_id) == setFieldIds.end()) {
                setFieldIds.insert(field->m_id);
            } else {
                return false;
            }
        }
    }
    jMsgOrderTypes(mapTypeNames, vecMessages);
    return true;
}

static bool jMsgTypeNameInVector(std::string& typeName, std::vector<JMsgType*>& vecMessages)
{
    for (size_t i = 0; i < vecMessages.size(); i++) {
        if (vecMessages[i]->m_typeName == typeName) {
            return true;
        }
    }
    return false;
}

static void jMsgAddTypeToVector(JMsgType* msgType, std::unordered_map<string, JMsgType*>& mapMessages, std::vector<JMsgType*>& vecMessages)
{
    if (jMsgTypeNameInVector(msgType->m_typeName, vecMessages)) {
        return;
    }

    for (size_t i = 0; i < msgType->m_vecFields.size(); i++) {
        JMsgField* field = msgType->m_vecFields[i];
        if (isBasicType(field->m_type)) {
            continue;
        }

        JMsgType* subType = mapMessages[field->m_type];
        jMsgAddTypeToVector(subType, mapMessages, vecMessages);
    }
    vecMessages.emplace_back(msgType);
}

static void jMsgOrderTypes(std::unordered_map<string, JMsgType*>& mapMessages, std::vector<JMsgType*>& vecMessages)
{
    vecMessages.clear();
    for (std::unordered_map<string, JMsgType*>::iterator iter = mapMessages.begin(); iter != mapMessages.end(); iter++) {
        jMsgAddTypeToVector(iter->second, mapMessages, vecMessages);
    }
}

bool jMsgIDLParse(const string& strData, std::vector<JMsgType*>& vecMessages)
{
    char* data = (char*)strData.c_str();

    do {
        JMsgType* msgType = nullptr;
        data = skipEmptyChars(data);
        data = skipComment(data);
        data = getType(data, &msgType);

        if (data) {
            vecMessages.emplace_back(msgType);
        } else {
            break;
        }
        data = skipEmptyChars(data);
    } while(*data != '\0');

    if (data) {
        return checkMessages(vecMessages);
    } else {
        return false;
    }
}
