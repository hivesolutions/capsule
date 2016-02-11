:: Hive Capsule Installer
:: Copyright (c) 2008-2016 Hive Solutions Lda.
::
:: This file is part of Hive Capsule Installer.
::
:: Hive Capsule Installer is free software: you can redistribute it and/or modify
:: it under the terms of the Apache License as published by the Apache
:: Foundation, either version 2.0 of the License, or (at your option) any
:: later version.
::
:: Hive Capsule Installer is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
:: Apache License for more details.
::
:: You should have received a copy of the Apache License along with
:: Hive Capsule Installer. If not, see <http://www.apache.org/licenses/>.

:: __author__    = João Magalhães <joamag@hive.pt>
:: __version__   = 1.0.0
:: __revision__  = $LastChangedRevision$
:: __date__      = $LastChangedDate$
:: __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
:: __license__   = Apache License, Version 2.0

capsule clone setup.exe
capsule append setup.exe dummy "Dummy File" http://127.0.0.1:9090/dummy.tar
setup dump dump.dat
