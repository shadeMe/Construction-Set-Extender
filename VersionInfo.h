#ifndef __CSE_VERSION_H__
#define __CSE_VERSION_H__

// these have to be macros so they can be used in the .rc
#define CSE_VERSION_INTEGER			6
#define CSE_VERSION_INTEGER_MINOR	0
#define CSE_VERSION_INTEGER_BETA	8
#define CSE_VERSION_VERSTRING		"6, 0, 8, 0"
#define CSE_VERSION_PADDEDSTRING	"0001"

#define MAKE_CSE_VERSION_EX(major, minor, build, sub)	(((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((build & 0xFFF) << 4) | (sub & 0xF))
#define MAKE_CSE_VERSION(major, minor, build)			MAKE_CSE_VERSION_EX(major, minor, build, 0)

#define PACKED_CSE_VERSION		MAKE_CSE_VERSION(CSE_VERSION_INTEGER, CSE_VERSION_INTEGER_MINOR, CSE_VERSION_INTEGER_BETA)

#endif /* __CSE_VERSION_H__ */
