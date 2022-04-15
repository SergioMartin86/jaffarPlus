extern void register_mapper_74x161x162x32();
extern void register_mapper_180();
extern void register_mapper_193();
extern void register_mapper_234();
extern void register_mapper_235();
extern void register_mapper_240();
extern void register_mapper_241();
extern void register_mapper_244();
extern void register_mapper_246();
extern void register_mapper_avenina();
extern void register_mapper_irem_g101();
extern void register_mapper_irem_tamS1();
extern void register_mapper_jaleco_jf11();
extern void register_mapper_namco_34xx();
extern void register_mapper_sunsoft1();
extern void register_mapper_sunsoft2();
extern void register_mapper_taito_tc0190();
extern void register_mapper_taito_x1005();
extern void register_mapper_un1rom();

void register_extra_mappers();
void register_extra_mappers()
{
	// Mapper 32
	register_mapper_irem_g101();

	// Mapper 33
	register_mapper_taito_tc0190();

	// Mapper 79, 113
	register_mapper_avenina();

	// Mapper 94
	register_mapper_un1rom();

	// Mapper 97
	register_mapper_irem_tamS1();

	// Mapper 140
	register_mapper_jaleco_jf11();

	// Mapper 184
	register_mapper_sunsoft1();

	// Mapper 89, 93
	register_mapper_sunsoft2();

	// Mapper 86, 152
	register_mapper_74x161x162x32();

	// Mapper 180
	register_mapper_180();

	// Mapper 193
	register_mapper_193();

	// Mapper 207
	register_mapper_taito_x1005();

	// Mapper 088, 154, 206
	register_mapper_namco_34xx();

	// Mapper 240
	register_mapper_240();

	// Mapper 241
	register_mapper_241();

	// Mapper 244
	register_mapper_244();

	// Mapper 246
	register_mapper_246();
}
