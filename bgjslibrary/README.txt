
Find good v8 version:
https://gist.github.com/domenic/aca7774a5d94156bfcc1

https://omahaproxy.appspot.com/
https://chromium.googlesource.com/v8/v8.git/+/branch-heads/5.4

Building v8:
1. get depot_tools
2. `fetch v8`
3. `git checkout origin/4.7.49`
4. `echo "target_os = ['android']" >> ../.gclient && gclient sync --nohooks` as described on wiki
5. `sudo apt-get install libc6-dev-i386 g++-multilib` (because compiling errors with some missed libraries)
6. add `'standalone_static_library': 1` to build/standalone.gypi (here) - to get a no-'thin library'
7. `make android_arm.release i18nsupport=off component=static_library -j 6`



https://stackoverflow.com/users/login?ssrc=head&returnurl=http%3a%2f%2fstackoverflow.com%2fquestions%2f25554621%2fturn-thin-archive-into-normal-one:

for lib in `find -name '*.a'`;
    do ar -t $lib | xargs ar rvs $lib.new && mv -v $lib.new $lib;
done

ndk-build
../gradlew assembleArmDebug
../gradlew assembleX86Debug

List of non-neon devices:
https://forum.unity3d.com/threads/failure-to-initialize-your-hardware-does-not-support-this-application-sorry.311613/#post-2139480
