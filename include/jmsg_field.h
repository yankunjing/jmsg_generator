#ifndef JMSG_FIELD_H
#define JMSG_FIELD_H
#include <string>
class JMsgField {
public:
    std::string m_name;
    std::string m_type;
    bool m_isArray = false;
    int m_id = 0;
    int m_typeId = 0;
};
#endif
