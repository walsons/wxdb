#ifndef ROW_H_
#define ROW_H_

/*********
 *  Row  *
 *********/
#define USER_NAME_MAX_LENGTH 31
#define EMAIL_MAX_LENGTH 255
struct Row 
{
    unsigned id;
    char user_name[USER_NAME_MAX_LENGTH + 1];
    char email[EMAIL_MAX_LENGTH + 1];
};

#define size_of_attribure(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
const unsigned kIdSize = size_of_attribure(Row, id);
const unsigned kUserNameSize = size_of_attribure(Row, user_name);
const unsigned kEmailSize = size_of_attribure(Row, email);
const unsigned kIdOffset = 0;
const unsigned kUserNameOffset = kIdOffset + kIdSize;
const unsigned kEmailOffset = kUserNameOffset + kUserNameSize;

#endif