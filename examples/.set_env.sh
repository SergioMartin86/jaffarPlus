if [[ -z "${JAFFAR2_MAX_WORKER_FRAME_DATABASE_SIZE_MB}" ]]; then
  export JAFFAR2_MAX_WORKER_FRAME_DATABASE_SIZE_MB=50
fi

if [[ -z "${JAFFAR2_HASH_AGE_THRESHOLD}" ]]; then
  export JAFFAR2_HASH_AGE_THRESHOLD=50
fi

if [[ -z "${JAFFAR2_SAVE_BEST_EVERY_SECONDS}" ]]; then
  export JAFFAR2_SAVE_BEST_EVERY_SECONDS=1
fi

if [[ -z "${JAFFAR2_SAVE_CURRENT_EVERY_SECONDS}" ]]; then
  export JAFFAR2_SAVE_CURRENT_EVERY_SECONDS=1
fi

if [[ -z "${JAFFAR2_SHOW_UPDATE_EVERY_SECONDS}" ]]; then
  export JAFFAR2_SHOW_UPDATE_EVERY_SECONDS=1
fi

if [[ -z "${JAFFAR2_SAVE_BEST_PATH}" ]]; then
  export JAFFAR2_SAVE_BEST_PATH=$HOME/jaffar.best.sav
fi

if [[ -z "${JAFFAR2_SAVE_CURRENT_PATH}" ]]; then
  export JAFFAR2_SAVE_CURRENT_PATH=$HOME/jaffar.current.sav
fi

#if [[ -z "${OMP_NUM_THREADS}" ]]; then
  export OMP_NUM_THREADS=6
#fi
