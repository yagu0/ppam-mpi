void t_serialize1();
void t_serialize2();
void t_deserialize1();
void t_deserialize2();
void t_compute_coefficients1();
void t_compute_coefficients2();
void t_pam1();
void t_pam2();
void t_pack1();
void t_pack2();
void t_unpack1();
void t_unpack2();
void t_rng1();
void t_rng2();
void t_utils1();
void t_utils2();

// list all unit tests here
int main()
{
	t_serialize1();
	t_serialize2();
	t_deserialize1();
	t_deserialize2();
	//~ t_compute_coefficients1();
	//~ t_compute_coefficients2();
	t_pam1(); //may fail
	t_pam2();
	t_pack1();
	t_pack2();
	t_unpack1();
	t_unpack2();
	t_rng1(); //may fail
	t_rng2();
	t_utils1();
	t_utils2();

	return 0;
}
