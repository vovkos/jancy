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

if [ "$DEPLOY_JANCY_PACKAGE" != "ON" ]; then
	return
fi

openssl \
	aes-256-cbc \
	-K $encrypted_98788fbcf1f3_key \
	-iv $encrypted_98788fbcf1f3_iv \
	-in $TRAVIS_BUILD_DIR/ci/travis/travis_id_rsa.enc \
	-out travis_id_rsa \
	-d

chmod 600 travis_id_rsa

eval $(ssh-agent -s)
ssh-add travis_id_rsa
ssh-keyscan github.com >> ~/.ssh/known_hosts

git clone git@github.com:vovkos/jancy-package

pushd jancy-package
git checkout ${TRAVIS_OS_NAME}-${TARGET_CPU}

cp $TRAVIS_BUILD_DIR/build/jancy-${TRAVIS_OS_NAME}-${TARGET_CPU}.tar.xz ./

git add --all
git commit --amend --message "Travis CI auto-deploy for: ${TRAVIS_COMMIT}"
git push --force

popd