#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

PACKAGE_SUFFIX=$1
PACKAGE_NAME=jancy-$PACKAGE_SUFFIX
PACKAGE_FILE=$PACKAGE_NAME.tar.xz

mkdir -p ~/.ssh
echo "$JANCY_PACKAGE_ID_RSA" > ~/.ssh/id_rsa
chmod 600 ~/.ssh/id_rsa

pushd build
cpack -G TXZ -D CPACK_PACKAGE_FILE_NAME=$PACKAGE_NAME
popd

git clone git@github.com:vovkos/jancy-package
pushd jancy-package
git checkout $PACKAGE_SUFFIX

cp ../build/$PACKAGE_FILE ./

git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
git config user.name "GitHub Actions"
git commit --all --amend --message "GitHub Actions auto-deploy for: $GITHUB_SHA"
git push --force
popd
