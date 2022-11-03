struct SufamiTurboCartridge {
  auto unload() -> void;
  auto power() -> void;
  auto serialize(serializer&) -> void;

  uint pathID = 0;
  ReadableMemory rom;
  WritableMemory ram;
};

thread_local extern SufamiTurboCartridge sufamiturboA;
thread_local extern SufamiTurboCartridge sufamiturboB;
