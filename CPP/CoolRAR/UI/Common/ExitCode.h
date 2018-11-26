// ExitCode.h

#ifndef __EXIT_CODE_H
#define __EXIT_CODE_H

namespace NExitCode {

enum EEnum {

  kSuccess       = 0,     // Successful operation
  kWarning       = 1,     // Non fatal error(s) occurred
  kFatalError    = 2,     // A fatal error occurred
  kUserError     = 7,     // Command line option error
  kMemoryError   = 8,     // Not enough memory for operation
  kUserBreak     = 255   // User stopped the process

};

}

#endif
