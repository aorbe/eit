##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Supervisor
ConfigurationName      :=Debug
WorkspacePath          := "/home/asantos/git/eit_code"
ProjectPath            := "/home/asantos/git/eit_code/Supervisor"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=André Santos
Date                   :=27/12/15
CodeLitePath           :="/home/asantos/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="Supervisor.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  `wx-config --libs`
IncludePath            :=  $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)dl $(LibrarySwitch)usb-1.0 $(LibrarySwitch)glut 
ArLibs                 :=  "dl" "usb-1.0" "glut" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)/lib/x86_64-linux-gnu/ $(LibraryPathSwitch)/usr/lib/x86_64-linux-gnu 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g -W `wx-config --cppflags` $(Preprocessors)
CFLAGS   :=  -g -W `wx-config --cflags` $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/ChSerialPort.cpp$(ObjectSuffix) $(IntermediateDirectory)/ChSimPort.cpp$(ObjectSuffix) $(IntermediateDirectory)/ChUsbPort.cpp$(ObjectSuffix) $(IntermediateDirectory)/ProtBasicBus.cpp$(ObjectSuffix) $(IntermediateDirectory)/ProtModbus.cpp$(ObjectSuffix) $(IntermediateDirectory)/Record.cpp$(ObjectSuffix) $(IntermediateDirectory)/Scan.cpp$(ObjectSuffix) $(IntermediateDirectory)/StDatabase.cpp$(ObjectSuffix) $(IntermediateDirectory)/Supervisor.cpp$(ObjectSuffix) $(IntermediateDirectory)/WConfig.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/WMain.cpp$(ObjectSuffix) $(IntermediateDirectory)/WMonitor.cpp$(ObjectSuffix) $(IntermediateDirectory)/math_mathplot.cpp$(ObjectSuffix) $(IntermediateDirectory)/sqlite_sqlite3.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/ChSerialPort.cpp$(ObjectSuffix): ChSerialPort.cpp $(IntermediateDirectory)/ChSerialPort.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/ChSerialPort.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ChSerialPort.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ChSerialPort.cpp$(DependSuffix): ChSerialPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ChSerialPort.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ChSerialPort.cpp$(DependSuffix) -MM "ChSerialPort.cpp"

$(IntermediateDirectory)/ChSerialPort.cpp$(PreprocessSuffix): ChSerialPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ChSerialPort.cpp$(PreprocessSuffix) "ChSerialPort.cpp"

$(IntermediateDirectory)/ChSimPort.cpp$(ObjectSuffix): ChSimPort.cpp $(IntermediateDirectory)/ChSimPort.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/ChSimPort.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ChSimPort.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ChSimPort.cpp$(DependSuffix): ChSimPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ChSimPort.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ChSimPort.cpp$(DependSuffix) -MM "ChSimPort.cpp"

$(IntermediateDirectory)/ChSimPort.cpp$(PreprocessSuffix): ChSimPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ChSimPort.cpp$(PreprocessSuffix) "ChSimPort.cpp"

$(IntermediateDirectory)/ChUsbPort.cpp$(ObjectSuffix): ChUsbPort.cpp $(IntermediateDirectory)/ChUsbPort.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/ChUsbPort.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ChUsbPort.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ChUsbPort.cpp$(DependSuffix): ChUsbPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ChUsbPort.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ChUsbPort.cpp$(DependSuffix) -MM "ChUsbPort.cpp"

$(IntermediateDirectory)/ChUsbPort.cpp$(PreprocessSuffix): ChUsbPort.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ChUsbPort.cpp$(PreprocessSuffix) "ChUsbPort.cpp"

$(IntermediateDirectory)/ProtBasicBus.cpp$(ObjectSuffix): ProtBasicBus.cpp $(IntermediateDirectory)/ProtBasicBus.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/ProtBasicBus.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ProtBasicBus.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ProtBasicBus.cpp$(DependSuffix): ProtBasicBus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ProtBasicBus.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ProtBasicBus.cpp$(DependSuffix) -MM "ProtBasicBus.cpp"

$(IntermediateDirectory)/ProtBasicBus.cpp$(PreprocessSuffix): ProtBasicBus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ProtBasicBus.cpp$(PreprocessSuffix) "ProtBasicBus.cpp"

$(IntermediateDirectory)/ProtModbus.cpp$(ObjectSuffix): ProtModbus.cpp $(IntermediateDirectory)/ProtModbus.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/ProtModbus.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ProtModbus.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ProtModbus.cpp$(DependSuffix): ProtModbus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ProtModbus.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ProtModbus.cpp$(DependSuffix) -MM "ProtModbus.cpp"

$(IntermediateDirectory)/ProtModbus.cpp$(PreprocessSuffix): ProtModbus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ProtModbus.cpp$(PreprocessSuffix) "ProtModbus.cpp"

$(IntermediateDirectory)/Record.cpp$(ObjectSuffix): Record.cpp $(IntermediateDirectory)/Record.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/Record.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Record.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Record.cpp$(DependSuffix): Record.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Record.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Record.cpp$(DependSuffix) -MM "Record.cpp"

$(IntermediateDirectory)/Record.cpp$(PreprocessSuffix): Record.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Record.cpp$(PreprocessSuffix) "Record.cpp"

$(IntermediateDirectory)/Scan.cpp$(ObjectSuffix): Scan.cpp $(IntermediateDirectory)/Scan.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/Scan.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Scan.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Scan.cpp$(DependSuffix): Scan.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Scan.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Scan.cpp$(DependSuffix) -MM "Scan.cpp"

$(IntermediateDirectory)/Scan.cpp$(PreprocessSuffix): Scan.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Scan.cpp$(PreprocessSuffix) "Scan.cpp"

$(IntermediateDirectory)/StDatabase.cpp$(ObjectSuffix): StDatabase.cpp $(IntermediateDirectory)/StDatabase.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/StDatabase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/StDatabase.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/StDatabase.cpp$(DependSuffix): StDatabase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/StDatabase.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/StDatabase.cpp$(DependSuffix) -MM "StDatabase.cpp"

$(IntermediateDirectory)/StDatabase.cpp$(PreprocessSuffix): StDatabase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/StDatabase.cpp$(PreprocessSuffix) "StDatabase.cpp"

$(IntermediateDirectory)/Supervisor.cpp$(ObjectSuffix): Supervisor.cpp $(IntermediateDirectory)/Supervisor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/Supervisor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Supervisor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Supervisor.cpp$(DependSuffix): Supervisor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Supervisor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Supervisor.cpp$(DependSuffix) -MM "Supervisor.cpp"

$(IntermediateDirectory)/Supervisor.cpp$(PreprocessSuffix): Supervisor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Supervisor.cpp$(PreprocessSuffix) "Supervisor.cpp"

$(IntermediateDirectory)/WConfig.cpp$(ObjectSuffix): WConfig.cpp $(IntermediateDirectory)/WConfig.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/WConfig.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/WConfig.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/WConfig.cpp$(DependSuffix): WConfig.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/WConfig.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/WConfig.cpp$(DependSuffix) -MM "WConfig.cpp"

$(IntermediateDirectory)/WConfig.cpp$(PreprocessSuffix): WConfig.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/WConfig.cpp$(PreprocessSuffix) "WConfig.cpp"

$(IntermediateDirectory)/WMain.cpp$(ObjectSuffix): WMain.cpp $(IntermediateDirectory)/WMain.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/WMain.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/WMain.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/WMain.cpp$(DependSuffix): WMain.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/WMain.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/WMain.cpp$(DependSuffix) -MM "WMain.cpp"

$(IntermediateDirectory)/WMain.cpp$(PreprocessSuffix): WMain.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/WMain.cpp$(PreprocessSuffix) "WMain.cpp"

$(IntermediateDirectory)/WMonitor.cpp$(ObjectSuffix): WMonitor.cpp $(IntermediateDirectory)/WMonitor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/WMonitor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/WMonitor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/WMonitor.cpp$(DependSuffix): WMonitor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/WMonitor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/WMonitor.cpp$(DependSuffix) -MM "WMonitor.cpp"

$(IntermediateDirectory)/WMonitor.cpp$(PreprocessSuffix): WMonitor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/WMonitor.cpp$(PreprocessSuffix) "WMonitor.cpp"

$(IntermediateDirectory)/math_mathplot.cpp$(ObjectSuffix): math/mathplot.cpp $(IntermediateDirectory)/math_mathplot.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/math/mathplot.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/math_mathplot.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/math_mathplot.cpp$(DependSuffix): math/mathplot.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/math_mathplot.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/math_mathplot.cpp$(DependSuffix) -MM "math/mathplot.cpp"

$(IntermediateDirectory)/math_mathplot.cpp$(PreprocessSuffix): math/mathplot.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/math_mathplot.cpp$(PreprocessSuffix) "math/mathplot.cpp"

$(IntermediateDirectory)/sqlite_sqlite3.c$(ObjectSuffix): sqlite/sqlite3.c $(IntermediateDirectory)/sqlite_sqlite3.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/asantos/git/eit_code/Supervisor/sqlite/sqlite3.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/sqlite_sqlite3.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sqlite_sqlite3.c$(DependSuffix): sqlite/sqlite3.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/sqlite_sqlite3.c$(ObjectSuffix) -MF$(IntermediateDirectory)/sqlite_sqlite3.c$(DependSuffix) -MM "sqlite/sqlite3.c"

$(IntermediateDirectory)/sqlite_sqlite3.c$(PreprocessSuffix): sqlite/sqlite3.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sqlite_sqlite3.c$(PreprocessSuffix) "sqlite/sqlite3.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


