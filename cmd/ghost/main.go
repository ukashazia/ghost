package main

/*
#cgo LDFLAGS: -framework AppKit
#include "mnkinterface.h"
*/
import "C"
import (
	"flag"
	"log"

	"os"
	"os/signal"
	"syscall"
)

func main() {

	// delay between repeated activities
	activityInterval := flag.Int("activity-interval", 60, "Activity interval in seconds")

	// time to wait after last m & k event
	interruptDelay := flag.Int("interrupt-after", 300, "Interrupt after this many seconds of inactivity")
	flag.Parse()

	cActivityInterval := C.seconds_t(*activityInterval)
	cInterruptDelay := C.seconds_t(*interruptDelay)
	C.keepAlive(&cInterruptDelay, &cActivityInterval)

	var termChan = make(chan os.Signal, 1)
	signal.Notify(termChan, syscall.SIGTERM)

	<-termChan
	log.Println("Exiting ...")
	os.Exit(0)
}
