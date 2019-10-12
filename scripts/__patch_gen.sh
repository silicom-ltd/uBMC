
PATCH_PATH_MAIN=${PATCH_LOCATION}/${PATCH_NAME_MAIN}

DIFF_BASE=${BASE_ROOT}/patch_gen
mkdir -p ${DIFF_BASE}
SRC_ORIG=${DIFF_BASE}/${PKG_DIR_NAME}-orig
SRC_NEW=${BR2_ROOT}/output/build/${PKG_DIR_NAME}


fix_patch_patch()
{
	if [ -z $1 ]; then
		return
	fi
	svn info $1 &>/dev/null
	if [ ! $? -eq 0 ]; then
		return
	fi
	echo -e "\tRemoving patch patch file header from $(basename $1)"
	mkdir -p /dev/shm/pp
	pp_name=/dev/shm/pp/$(basename $1).pp.patch
	pp_rev_name=/dev/shm/pp/$(basename $1).pp_rev.patch
	svn diff $1 >$pp_name
	${BASE_ROOT}/scripts/patch_reduce.py ${pp_name} ${pp_rev_name}
	PWD_SAVE=$(pwd)
	cd / && patch -p0 -R <${pp_rev_name}
	cd ${PWD_SAVE}
}

if [ ! -d ${SRC_ORIG} ]; then
	if [ -f ${SRC_ORIG} ]; then
		echo "${SRC_ORIG} exists and is not a directory, removing ..."
		rm -rf ${SRC_ORIG}
	fi
	if [ ! -e ${SRC_ORIG} ]; then
		echo "${SRC_ORIG} doesn't exist, extracting new."
		mkdir -p ${SRC_ORIG}
		tar -xf ${BR2_ROOT}/dl/${PKG_TAR_NAME} --strip-components=1 -C ${SRC_ORIG} || rm -rf ${SRC_ORIG}
		if [ ! -d ${SRC_ORIG} ]; then
			echo "Failed to extrace original src ${BR2_ROOT}/dl/${PKG_TAR_NAME}"
			exit 1
		fi
		if [ ! -z "${PKG_PRE_PATCH_DIR}" ]; then
			cd ${SRC_ORIG}
			echo "Package have pre patch"
			for ppp in $(ls ${PKG_PRE_PATCH_DIR}/*.patch); do 
				if [ ! -z "${PATCH_NAME_MAIN}" ]; then
					if [ $(basename $ppp) == ${PATCH_NAME_MAIN} ]; then
						continue;
					fi
				fi
				OUR_PATCH=0
				for mmm in $(seq 1 100); do
					if [ -z "${PATCH_NAME[$mmm]}" ]; then break; fi
					if [ $(basename $ppp) == ${PATCH_NAME[$mmm]} ]; then OUR_PATCH=1; break; fi
				done
				if [ ${OUR_PATCH} -eq 1 ]; then continue; fi
				echo "Apply pre patch $ppp to original source"
				patch -p1 <$ppp
			done
		fi
	fi
fi

#set -x

(	cd ${DIFF_BASE};
	if [ ! -e "${PKG_DIR_NAME}-silicom" ]; then
		ln -f -s ${SRC_NEW} ${PKG_DIR_NAME}-silicom
	fi
	if [ ! -z "${PATCH_GEN_REQ_CLEAN}" ]; then
		if [ ${PATCH_GEN_REQ_CLEAN} -eq 1 ]; then
			echo "Clean new source"
			cd ${PKG_DIR_NAME}-silicom
			make clean
			cd ..
		fi
	fi

	if [ ! -z "${PATCH_GEN_REQ_DIST_CLEAN}" ]; then
		if [ ${PATCH_GEN_REQ_DIST_CLEAN} -eq 1 ]; then
			echo "Dist Clean new source"
			cd ${PKG_DIR_NAME}-silicom
			if [ -f Makefile ]; then
				make distclean
			else
				echo "Makfile not found, already distcleaned?"
			fi
			cd ..
		fi
	fi

	for mmm in $(seq 1 100); do
		if [ -z "${PATCH_NAME[$mmm]}" ]; then
			break
		fi
		if [ -z "${PATCH_FLIST[$mmm]}" ]; then
			echo "Patch file list file not defined for ${PATCH_NAME[$mmm]}"
			exit 1
		fi
		if [ ! -f ${PATCH_BASE}/${PATCH_FLIST[$mmm]} ]; then
			echo "Patch file list file ${PATCH_BASE}/${PATCH_FLIST[$mmm]} not a regular file"
			exit 1
		fi
	done
	if [ ! -z "${PATCH_NAME_MAIN}" ] ; then
		if [ -z "${PATCH_MAIN_EXCEPT_FILE}" ]; then
			echo "Patch file list file not defined for ${PATCH_NAME_MAIN}"
			exit 1
		fi
		if [ ! -f ${PATCH_BASE}/${PATCH_MAIN_EXCEPT_FILE} ]; then
			echo "Patch file list file ${PATCH_BASE}/${PATCH_MAIN_EXCEPT_FILE} not a regular file"
			exit 1
		fi
		echo "Generating main patch ${PATCH_PATH_MAIN}"
		echo "diff -uNr ${PKG_DIR_NAME}-orig ${PKG_DIR_NAME}-silicom -X ${PATCH_BASE}/${PATCH_MAIN_EXCEPT_FILE}> ${PATCH_PATH_MAIN}"
		diff -uNr ${PKG_DIR_NAME}-orig ${PKG_DIR_NAME}-silicom -X ${PATCH_BASE}/${PATCH_MAIN_EXCEPT_FILE}> ${PATCH_PATH_MAIN}
		fix_patch_patch ${PATCH_PATH_MAIN}
	fi
	for mmm in $(seq 1 100); do
		if [ -z "${PATCH_NAME[$mmm]}" ]; then
			break
		fi
		PATCH_PATH[$mmm]=${PATCH_LOCATION}/${PATCH_NAME[$mmm]}
		echo "Generating patch $mmm ${PATCH_PATH[$mmm]}"
		
		rm -rf ${PATCH_PATH[$mmm]}
		for patch_fname in $(cat ${PATCH_BASE}/${PATCH_FLIST[$mmm]}); do
			echo -e "\tfile ${patch_fname}"
			if [ -z "$(echo ${patch_fname}|grep /)" ]; then
				for fname in $(find ${PKG_DIR_NAME}-silicom/ -name ${patch_fname}|cut -d / -f 2-); do 
					echo -e "\t\tfound ${patch_fname}"
					echo "diff -uNr ${PKG_DIR_NAME}-orig/${fname} ${PKG_DIR_NAME}-silicom/${fname} >>${PATCH_PATH[$mmm]}"
					diff -uNr ${PKG_DIR_NAME}-orig/${fname} ${PKG_DIR_NAME}-silicom/${fname} >>${PATCH_PATH[$mmm]}
				done
			else
				fname=$(echo ${patch_fname}|sed "s#^/##")
				echo "diff -uNr ${PKG_DIR_NAME}-orig/${fname} ${PKG_DIR_NAME}-silicom/${fname} >>${PATCH_PATH[$mmm]}"
				diff -uNr ${PKG_DIR_NAME}-orig/${fname} ${PKG_DIR_NAME}-silicom/${fname} >>${PATCH_PATH[$mmm]}
			fi
		done
		fix_patch_patch ${PATCH_PATH[$mmm]}
	done
)
	if [ ! -z "${PATCH_NAME_MAIN}" ]; then
		if [ ! -z "$(cat ${PATCH_PATH_MAIN}|grep Binary|grep ' differ')" ]; then
			echo "Generated pacth contains binary files that are not included"
			cat ${PATCH_PATH_MAIN}|grep Binary|grep " differ"
		fi
	fi

