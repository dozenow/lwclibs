import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.concurrent.Semaphore;

public class IOTest_TDs implements Runnable {

	static {
		System.loadLibrary("testiojni");
	}

	private static int numTDs = 4;

	private static int sync = 0;
	private static Semaphore sem = new Semaphore(1, true);

	int tid;

	public IOTest_TDs(int tid) {
		this.tid = tid;
	}

	public void run() {

		LWCNI.lwCConfine();

		System.out.println("Thread " + tid + " is awake");

		try {

			// create 1000 temporary files
			for (int i = 0; i < 1000; i++) {

				if (sync == 1)
					sem.acquire();

				BufferedWriter out = new BufferedWriter(new FileWriter("tmpfiles/" + tid + "/" + i + ".iotest.txt"));
				out.write("Refmon, are you getting this?");
				out.close();

				if (sync == 1)
					sem.release();
			}

			// read and double check
			for (int i = 0; i < 1000; i++) {
				BufferedReader in = new BufferedReader(new FileReader("tmpfiles/" + tid + "/" + i + ".iotest.txt"));
				String l = in.readLine();
				assert (l.compareTo("Refmon, are you getting this?") == 0);
				in.close();
			}

			// remove
			for (int i = 0; i < 1000; i++) {
				new File("tmpfiles/" + tid + "/" + i + ".iotest.txt").delete();
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static void main(String[] args) throws IOException {

		// main thread and helpers
		LWCNI.lwCRegister(1 + numTDs);

		Thread[] tds = new Thread[numTDs];

		// create threads
		for (int i = 0; i < numTDs; i++) {
			tds[i] = new Thread(new IOTest_TDs(i));
			tds[i].start();
		}

		// confining after jvm init
		System.out.println("Confining after init");
		int rmfd = LWCNI.lwCConfine();

		for (int i = 0; i < numTDs; i++) {
			try {
				tds[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		System.out.println("Test done!");
		LWCNI.lwCCleanup(rmfd);
	}
}
