import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupPremake import PremakeConfiguration as PremakeRequirements
from SetupVulkan import VulkanConfiguration as VulkanRequirements
os.chdir('./../') # Change from devtools/scripts directory to root

premakeInstalled = PremakeRequirements.Validate()
VulkanRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if premakeInstalled:
    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call(["vendor/premake/bin/premake5.exe", "vs2022"])

    print("\nSetup completed!")
else:
    print("Hazel requires Premake to generate project files.")

