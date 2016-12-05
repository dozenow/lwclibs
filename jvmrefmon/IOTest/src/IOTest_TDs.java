import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class IOTest_TDs implements Runnable {

	static {
		System.loadLibrary("testiojni");
	}

	int tid;

	public IOTest_TDs(int tid) {
		this.tid = tid;
	}

	public void run() {

		try {
			// create 100 temporary files
			for (int i = 0; i < 10000; i++) {
				BufferedWriter out = new BufferedWriter(
						new FileWriter("tmpfiles/" + tid + "/" + i + ".iotest.txt"));
				out.write("Refmon, are you getting this?");
				out.close();
			}
			// read and double check
			for (int i = 0; i < 10000; i++) {
				BufferedReader in = new BufferedReader(new FileReader("tmpfiles/" + tid + "/" + i + ".iotest.txt"));
				String l = in.readLine();
				assert (l.compareTo("Refmon, are you getting this?") == 0);
				in.close();
			}

			// remove
			for (int i = 0; i < 10000; i++) {
				new File("tmpfiles/" + tid + "/" + i + ".iotest.txt").delete();
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private static int numTDs = 1;

	public static void main(String[] args) throws IOException {

		Thread[] tds = new Thread[numTDs];

		// create threads
		for (int i = 0; i < numTDs; i++) {
			tds[i] = new Thread(new IOTest_TDs(i));
		}

		//confining after jvm init
		System.out.println("Confining after init");
		IOTest.lwCConfine();
		IOTest.lwCRegister();
		
		System.out.println("Start test");
		for (int i = 0; i < numTDs; i++) {
			tds[i].start();
		}

		for (int i = 0; i < numTDs; i++) {
			try {
				tds[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		
		System.out.println("Test done!");
		IOTest.lwCCleanup();
	}
}
