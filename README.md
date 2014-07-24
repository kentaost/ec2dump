# ec2dump
EC2 crash dump tool instead of kdump. 

## Overview
* ec2dump is an implementation of crash dump for an environment which cannot use kdump.
* Don't use this tool for your production environment. This tool is just only for test now.
* To analyze core dump made by this tool needs to modify crash program.  
Patch(crash-7.0.7.vol.patch) is from:  
http://www.4tphi.net/~awalters/dfrws2008/volcrash-4.0-6.3_patch

## Developer
Kenta Tada  
e-mail: ktagml@gmail.com
