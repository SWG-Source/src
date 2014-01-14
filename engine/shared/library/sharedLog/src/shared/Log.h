// ======================================================================
//
// Log.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Log_H
#define INCLUDED_Log_H

// ======================================================================

#include "LogManager.h"

// ======================================================================

// Un-conditional

#define LOG(stream, varArgs) (LogManager::setArgs(stream), LogManager::log varArgs)
#define LOGU(stream, varArgs, unicodeAttach) (LogManager::setArgs(stream, unicodeAttach), LogManager::log varArgs)

// Conditional

#define LOGC(forceLog, stream, varArgs) ((forceLog) ? (LogManager::setArgs(stream), LogManager::log varArgs) : NOP)
#define LOGUC(forceLog, stream, varArgs, unicodeAttach) ((forceLog) ? (LogManager::setArgs(stream, unicodeAttach), LogManager::log varArgs) : NOP)

#ifdef _DEBUG
#define DEBUG_LOG(stream, varArgs) LOG(stream, varArgs)
#define DEBUG_LOGU(stream, varArgs, unicodeAttach) LOGU(stream, varArgs, unicodeAttach)
#else
#define DEBUG_LOG(stream, varArgs) NOP
#define DEBUG_LOGU(stream, varArgs, unicodeAttach) NOP
#endif

// ======================================================================

#endif

