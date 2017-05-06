package config

import "bytes"

const (
	PermRead = 0x1 << 2
	PermWrite = 0x1 << 1
	PermInherit = 0x1
)

func Perm2String(perm int) string {
	var strBuf bytes.Buffer
	if Readable(perm) {
		strBuf.WriteString("R")
	} else {
		strBuf.WriteString("-")
	}

	if Writeable(perm) {
		strBuf.WriteString("W")
	} else {
		strBuf.WriteString("-")
	}

	if Inherited(perm) {
		strBuf.WriteString("X")
	} else {
		strBuf.WriteString("-")
	}

	return strBuf.String()
}

func Readable(perm int) bool {
	return (perm & PermRead) == PermRead
}

func Writeable(perm int) bool{
	return (perm & PermWrite) == PermWrite
}

func Inherited(perm int) bool{
	return (perm & PermInherit) == PermInherit
}

const Separator = " "

var defaultReadQueueNum int = 16
var defaultWriteQueueNums int = 16

type TopicFilterType int

const (
	SingleTag TopicFilterType = iota
	MultiTag
)

func (t TopicFilterType) String() string {
	if t == SingleTag {
		return "SINGLE_TAG"
	}
	return "MULTI_TAG"
}

type TopicConfig  struct {
	TopicName string
	ReadQueueNum int
	WriteQueueNum int
	Perm int
	TopicFilter TopicFilterType
	TopicSysFlag int
	Order bool
}

func NewTopicConfig(topicName string, readQueueNum, writeQueueNum, perm int) TopicConfig {
	cfg := TopicConfig{
		TopicName: topicName,
		ReadQueueNum: readQueueNum,
		WriteQueueNum: writeQueueNum,
		Perm: perm,
		TopicFilter: SingleTag,
		TopicSysFlag: 0,
		Order: false,
	}

	if perm == 0 {
		cfg.Perm = PermRead | PermWrite
	}

	return cfg
}

func (cfg *TopicConfig) Encode() string
func (cfg *TopicConfig) Decode(in string) bool
func (cfg *TopicConfig) String() string
