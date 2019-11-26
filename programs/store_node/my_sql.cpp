  


#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>
#include <unistd.h> 
#include <fc/exception/exception.hpp> 
#include <fc/interprocess/signals.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>
#include <boost/filesystem.hpp> 
#include <graphene/app/application.hpp>

#include <iostream>
#include <fstream>

#ifdef WIN32
# include <signal.h>
#else
# include <csignal>
#endif

//export MYSQL_ROOT_DIR=/usr/local/include
//export MYSQL_INCLUDE_DIR=/usr/local/include
 
using namespace graphene;
namespace bpo = boost::program_options;

std::string my_db_url = "";
std::string my_db_user = "";
std::string my_db_pwd = "";
std::string my_db_name = "store_db";
int    my_db_port = 3306;
MYSQL my_sql_hanele;
MYSQL_RES *my_sql_res = nullptr;

bool connect_my_sql()
{
   if(!mysql_real_connect(&my_sql_hanele,my_db_url.c_str(),my_db_user.c_str(),my_db_pwd.c_str(),my_db_name.c_str(),my_db_port,NULL,0))
   {    
       elog("ERROR: fail to url ${x} user  ${y}  errorno ${z} error  ${a}  ", ("x", my_db_url)("y", my_db_user)("z",mysql_errno(&my_sql_hanele))("a",mysql_error(&my_sql_hanele)));
        return false;
   }
   ilog("sql connect OK! url ${x} user  ${y} ", ("x", my_db_url)("y", my_db_user));
   return true;
}

bool init_my_sql(bpo::variables_map options)
{   
   if (options.count("store-db-url")) {
      my_db_url = options["store-db-url"].as<std::string>();
   }
   if (options.count("store-db-user")) {
      my_db_user = options["store-db-user"].as<std::string>();
   }
   if (options.count("store-db-pwd")) {       
      my_db_pwd = options["store-db-pwd"].as<std::string>();
   }
   if (options.count("store-db-name")) {       
      my_db_name = options["store-db-name"].as<std::string>();
   }  
   if (options.count("store-db-port")) {
      my_db_port = options["store-db-port"].as<int>();
   }
   if(my_db_url.size()<3 || my_db_user.size()<1)
   {
       FC_ASSERT(false, "connect my sql failed url ${x} user  ${y} ", ("x", my_db_url)("y", my_db_user));
       return false;
   }
   return connect_my_sql();
}

void release_my_sql()
{  
  if(my_sql_res)  
   mysql_free_result(my_sql_res);
  mysql_close(&my_sql_hanele);
}

void do_mysql_query(const std::string &sql_query)
{  
 next_do:
   int ret=mysql_query(&my_sql_hanele,sql_query.c_str());
   if(ret != 0)
   {
     if(mysql_errno(&my_sql_hanele) == 2006)
     {
        elog("sql query failed! errorno ${x} error  ${y} \n", ("x", mysql_errno(&my_sql_hanele))("y", mysql_error(&my_sql_hanele)));
        release_my_sql();
        fc::usleep(fc::seconds(5*60));
        connect_my_sql();
        goto next_do;
     }
     else
     {
       elog("sql query failed! errorno ${x} error  ${y} \n", ("x", mysql_errno(&my_sql_hanele))("y", mysql_error(&my_sql_hanele)));
       elog(sql_query.c_str());
       assert(0);
     }
   }
}
uint64_t store_db_start()
{    
    uint64_t ireturn = 0;
    MYSQL_ROW row;
    MYSQL_RES *res = nullptr;
    if (mysql_query(&my_sql_hanele, "select max(block_num) from block_summary_object")) 
    {
        elog(" store_db_start errorno ${x} error  ${y} ", ("x", mysql_errno(&my_sql_hanele))("y", mysql_error(&my_sql_hanele)));
        return 0;
    }
    res = mysql_use_result(&my_sql_hanele);    
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if(row[0]!=NULL)
        {
          ireturn = fc::to_int64(row[0]);
          break;
        }
    }
    mysql_free_result(res);
    elog(" store_db_start ok  ${x} ",("x", ireturn));
    return ireturn;
}