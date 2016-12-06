
public class LWCNI {

	/* should be called before confine, otherwise it will be assumed that numThreads = 1 */
	/* numThreads is the total number of threads that will ever trap into in the refmon */
	public native static void lwCRegister(int numThreads);
	
	/* creates a refmon context, to which the app context traps into */
	/* SYNC point!! all threads must call into */
	/* returns the fd of the refmon context */
	public native static int lwCConfine();

	/* cleanup */
	public native static void lwCCleanup(int rmfd);
}
