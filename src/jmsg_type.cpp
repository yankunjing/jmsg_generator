#include "jmsg_type.h"
#include "jmsg_field.h"
using namespace std;
JMsgField* JMsgType::getFieldById(int fieldId)
{
    for (JMsgField* field : m_vecFields) {
        if (field->m_id == fieldId) {
            return field;
        }
    }
    return nullptr;
}

JMsgField* JMsgType::getFieldByName( const string& fieldName )
{
    for (JMsgField* field : m_vecFields) {
        if(field->m_name == fieldName) {
            return field;
        }
    }
    return nullptr;
}
