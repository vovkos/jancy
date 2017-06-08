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

openssl \
	aes-256-cbc \
	-K $encrypted_98788fbcf1f3_key \
	-iv $encrypted_98788fbcf1f3_iv \
	-in $TRAVIS_BUILD_DIR/ci/travis/travis_id_rsa.enc \
	-out travis_id_rsa \
	-d

chmod 600 travis_id_rsa

sftp \
	-i travis_id_rsa \
	-o StrictHostKeyChecking=no \
	travis@jancy.org \
	<< ===

	lcd $TRAVIS_BUILD_DIR/build/bin/Release
	cd /incoming/jancy/bin
	put jancy

	cd /incoming/jancy/extensions
	rm *.jncx
	put *.jncx

	lcd $TRAVIS_BUILD_DIR/samples/jnc
	cd /incoming/jancy/samples
	rm *.jnc
	put *.jnc

	bye

===
