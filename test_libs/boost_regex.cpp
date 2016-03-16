/*************************************************************************
	> File Name: boost_regex.cpp
	> Author: gwq5210
	> Mail: gwq5210@qq.com 
	> Created Time: 2016年03月16日 星期三 20时38分48秒
 ************************************************************************/

#include <stdio.h>

#include <boost/regex.hpp>

using namespace boost;

int main(int argc, char *argv[])
{
    const char *mail_reg_str = "\\w+@\\w+.\\w+";
    char buf[1024];
    regex mail_reg(mail_reg_str);
    while (scanf("%s", buf) != EOF) {
        if (regex_match(buf, mail_reg)) {
            printf("\"%s\" is a mail address!\n", buf);
        } else {
            printf("\"%s\" is not a mail address!\n", buf);
        }
    }

	return 0;
}
