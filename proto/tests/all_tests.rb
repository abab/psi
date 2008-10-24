#! /usr/bin/ruby

CLEAN_COMMAND = 'make -s distclean';
QMAKE_COMMAND = '/usr/local/Trolltech/Qt-4.4.3/bin/qmake';
MAKE_COMMAND  = 'make -j 2 -s';

class Tester
	@@failCount = 0
	def failCount()
		@@failCount
	end

	def runTestCase(test)
		Dir.chdir(test) do
			runResult = system('./' + test + '_test')
			@@failCount += (runResult ? 0 : 1)
			puts
		end
	end

	def initialize()
		system(CLEAN_COMMAND)
		system(QMAKE_COMMAND)
		print("Recompiling tests... ")
		abort("Make error!") if (!system(MAKE_COMMAND))
		puts; puts
	end
end

tester = Tester.new()
tester.runTestCase('backend')
tester.runTestCase('models')
tester.runTestCase('xep82')

if(tester.failCount() == 0)
	puts "All tests passed."
else
	puts "Some tests failed!"
end
