
#include <string>
#include <sstream>

/**
 * @brief helper to generate a json-document from primary c-types
 * It separates each member with ',' but douse no string escaping.
 */
class JsonSerializer{
public:
    /**
     * @brief add a json-class to the document
     * 
     * @param classMemers inner json that holds the class member (seperated by ',')
     */
    void addClass(std::string classMemers){
        data << '{';
        data << classMemers;
        data << '}';
    }

    /**
     * @brief interpret this document as json-class definition
     * This can be used as class for a superier json.
     * 
     * @return std::string 
     */
    std::string asClass(){
        return '{'+ data.str() + '}';
    }

    /**
     * @brief add a member to this json document
     * It adds <member>:<value> to the json. 
     * Combine this with addClass or asClass for a valid json.
     * @tparam t the c-type it should be handled by std::stringsstream
     * @param member name of the member of a json-class
     * @param value the value of the member
     */
    template<typename t> void addMember(std::string member, t value){
        setMemberName(member);
        data << value;
    }

    /**
     * @brief add a member to this json document
     * It adds <member>:<True or False> to the json. 
     * Combine this with addClass or asClass for a valid json.
     * @param member name of the member of a json-class
     * @param value the value of the member
     */
    void addBoolMember(std::string member, bool value){
        setMemberName(member);
        setBool(value);
    }

    /**
     * @brief add a json-list as class member
     * It generates <member>:[value, value, ...]
     * @tparam iterator iteratable type the inner should be a c-type that is handled by std::stringsstream
     * @param member name of the member of a json-class
     * @param begin start of the array
     * @param end  end of the array
     */
    template<typename iterator>void addArray(std::string member, iterator begin, iterator end){
        setMemberName(member);
        setArray(begin, end);
    }

    /**
     * @brief add a json-list as class member
     * It generates <member>:[True/False, True/False, ...]
     * @tparam iterator iteratable type the inner should be a bool-type that is handled by std::stringsstream
     * @param member name of the member of a json-class
     * @param begin start of the array
     * @param end  end of the array
     */
    template<typename iterator>void addBoolArray(std::string member, iterator begin, iterator end){
        setMemberName(member);
        setBoolArray(begin, end);
    }

    /**
     * @brief add a json-list 
     * It generates [value, value, ...]
     * @tparam iterator iteratable type the inner should be a bool-type that is handled by std::stringsstream
     * @param begin start of the array
     * @param end  end of the array
     */
    template<typename iterator>void setArray(iterator begin, iterator end){
        data << '[';
        for(iterator itr = begin; itr!=(end-1); itr++){
            data<<*itr;
            data<<',';
        }
        if(begin != end-1){
            data << *(end-1);
        }
        data << ']';
    }

    /**
     * @brief add a json-list 
     * It generates [True/False, True/False, ...]
     * @tparam iterator iteratable type the inner should be a bool-type that is handled by std::stringsstream
     * @param begin start of the array
     * @param end  end of the array
     */
    template<typename iterator>void setBoolArray(iterator begin, iterator end){
        data << '[';
        for(iterator itr = begin; itr!=(end-1); itr++){
            setBool(*itr);
            data<<',';
        }
        if(begin != end-1){
            setBool(*(end-1));
        }
        data << ']';
    }

    /**
     * @brief get the json-date
     * @attention the user is responsible for genarating a valid json
     */
    std::string str(){
        return data.str();
    }
    
    std::stringstream data;

protected:
    bool firstMember = true;
    void setMemberName(std::string name){
        handleFirstMember();
        data << '"';
        data << name;
        data << "\": ";
    }
    void handleFirstMember(){
        if(firstMember){
            firstMember = false;
        }else{
            data << ',';
        }
    }

    void setBool(bool value){
        if(value){
            data << "true";
        }else{
            data <<"false";
        }
    }
    
};