





#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/transcoder.h>








#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>








namespace Util {

std::string logFileName(std::string const& appname) {



}

//TODO: free memory for valgrind?

void initLogger(std::string const& appname,









{







  log4cxx::LoggerPtr g_pStatsDBLogger(log4cxx::Logger::getRootLogger());

    g_pStatsDBLogger->removeAllAppenders();
  log4cxx::LogString logStrLayout;




  log4cxx::PatternLayout* pLayout = new log4cxx::PatternLayout(logStrLayout);




    log4cxx::LogString logFileName;


    g_pStatsDBLogger->addAppender(pAppender);
  }

    g_pStatsDBLogger->addAppender(
        new log4cxx::ConsoleAppender(pLayout, log4cxx::ConsoleAppender::getSystemErr())
                                  );
  }


  }
  g_pStatsDBLogger->setLevel(level);



}






























